#!/usr/bin/env python3
import subprocess
import sys
import os
import time

# ====================================================================
# TIMEOUT CONFIGURATION
# ====================================================================
PAPER_TIMEOUT_THRESHOLD = 60.0
MAXTIME = 120  # Explicit 120s ceiling for all tracking steps

# Table 3 explicit structures
summary_csv = "t3_multi_summary.csv"
log_files = {
    'equiv':  "t3_multi_equivchecking.csv",
    'model':  "t3_multi_modelchecking.csv",
    'pgsol':  "t3_multi_pgsolver.csv",
    'random': "t3_multi_random.csv"
}

domains_setup = [
    {'key': 'equiv',  'module_name': 'games_equivchecking',  'desc': 'Equivalence Checking'},
    {'key': 'model',  'module_name': 'games_modelchecking',  'desc': 'Model Checking'},
    {'key': 'pgsol',  'module_name': 'games_pgsolver',       'desc': 'PGSolver'},
    {'key': 'random', 'module_name': 'games_random',         'desc': 'RandomG'}
]

summary_matrix_cache = {}

def rewrite_summary_csv_live():
    """Rewrites the central Table 3 summary sheet with dynamic column matching."""
    with open(summary_csv, "w") as f:
        f.write("Algorithm,EquivcheckingTime,EquivcheckingSolved,ModelcheckingTime,ModelcheckingSolved,PGSolverTime,PGSolverSolved,RandomGTime,RandomGSolved\n")
        for algo_name, domain_data in summary_matrix_cache.items():
            row = [algo_name]
            for domain in domains_setup:
                stats = domain_data[domain['key']]
                if stats:  
                    row.extend([stats['time'], stats['solved']])
                else:      
                    row.extend(["", ""])
            f.write(",".join(row).rstrip(",") + "\n")

# Initialize Log Files cleanly if they don't exist yet
for path_log in log_files.values():
    if not os.path.exists(path_log):
        with open(path_log, "w") as f:
            pass 

# ====================================================================
# ALGORITHMS CONFIGURATION SUITE (TABLE 3 SPECIFIC MAPS)
# ====================================================================
# Use None placeholders for thresholds; they are injected dynamically per game iteration
algorithms_pool = [
    {'name': 'w',         'type': 'parity'},
    {'name': 'eta',       'type': 'energy'},
    {'name': 'mu',        'type': 'mean_payoff'},
    {'name': 'w x eta',   'type': 'parity_energy'},
    {'name': 'w x mu',    'type': 'parity_mean_payoff'},
    {'name': 'eta x mu',  'type': 'energy_mean_payoff'},
    {'name': 'w x eta x mu', 'type': 'all_constraints'}
]

# ====================================================================
# MAIN EVALUATION RUNNER ENGINE
# ====================================================================
for algo in algorithms_pool:
    print("=" * 85)
    print(f"Evaluating Table 3 Algorithm Row: {algo['name']} (Small + Large Merged)")
    print("=" * 85)
    
    summary_matrix_cache[algo['name']] = {d['key']: None for d in domains_setup}
    rewrite_summary_csv_live()

    for domain in domains_setup:
        key = domain['key']
        time_sum = 0.0
        solved = 0
        total = 0
        
        print(f"\n>>> Starting Domain: {domain['desc']}")
        
        try:
            g_module = __import__(domain['module_name'])
            base_path = g_module.path
            
            # Extract lists from configuration files safely
            f_small = getattr(g_module, 'files_small', [])
            f_large = getattr(g_module, 'files_large', [])
            
            i_small = getattr(g_module, 'inits_small', [])
            i_large = getattr(g_module, 'inits_large', [])
            
            e_small = getattr(g_module, 'thenergy_small', [])
            e_large = getattr(g_module, 'thenergy_large', [])
            
            m_small = getattr(g_module, 'thmean_small', [])
            m_large = getattr(g_module, 'thmean_large', [])
            
            # CONCATENATION STEP: Combine small and large runs into unified tracking pools
            files_list = f_small + f_large
            inits_list = i_small + i_large
            energy_list = e_small + e_large
            mean_list = m_small + m_large
            
            total = len(files_list)
            
            if total == 0:
                print(f"  [Info] No files loaded for {domain['module_name']}. Skipping domain.")
                summary_matrix_cache[algo['name']][key] = {'time': '', 'solved': ''}
                rewrite_summary_csv_live()
                continue

            # LIVE LOGGING: Write the algorithm token at start of log line
            with open(log_files[key], "a") as f:
                f.write(f"{algo['name']}")

            for idx, filename in enumerate(files_list):
                print(f"  [{idx+1}/{total}] Running {filename}...", end="", flush=True)
                
                # Fetch matching initialization and thresholds safely
                init_val = str(inits_list[idx]) if idx < len(inits_list) else "0"
                th_energy = str(energy_list[idx]) if idx < len(energy_list) else "0"
                th_mean = str(mean_list[idx]) if idx < len(mean_list) else "0"

                # Build variable flags dynamically matching row type
                flag_args = ["--chuffed", "--init", init_val, "--print-only-totaltime"]
                
                if algo['type'] == 'parity':
                    flag_args += ["--parity"]
                elif algo['type'] == 'energy':
                    flag_args += ["--energy", th_energy, "--weights", "-100", "100"]
                elif algo['type'] == 'mean_payoff':
                    flag_args += ["--mean-payoff", th_mean, "--weights", "-100", "100"]
                elif algo['type'] == 'parity_energy':
                    flag_args += ["--parity", "--energy", th_energy, "--weights", "-100", "100"]
                elif algo['type'] == 'parity_mean_payoff':
                    flag_args += ["--parity", "--mean-payoff", th_mean, "--weights", "-100", "100"]
                elif algo['type'] == 'energy_mean_payoff':
                    flag_args += ["--energy", th_energy, "--mean-payoff", th_mean, "--weights", "-100", "100"]
                elif algo['type'] == 'all_constraints':
                    flag_args += ["--parity", "--energy", th_energy, "--mean-payoff", th_mean, "--weights", "-100", "100"]

                # ------------------------------------------------------------
                # ASYNCHRONOUS PARALLEL RACE ENGINE (noc-even vs noc-odd)
                # ------------------------------------------------------------
                c_name_even = f"t3_race_even_{idx}"
                c_name_odd  = f"t3_race_odd_{idx}"

                cmd_even = ["docker", "run", "--rm", "--name", c_name_even, "--init", "-v", f"{base_path}:/mnt", "solver", 
                            "nocq", "--gm", f"/mnt/{filename}", "--noc-even"] + flag_args
                cmd_odd  = ["docker", "run", "--rm", "--name", c_name_odd, "--init", "-v", f"{base_path}:/mnt", "solver", 
                            "nocq", "--gm", f"/mnt/{filename}", "--noc-odd"] + flag_args
                
                p_even = None
                p_odd = None
                race_stdout = None
                parsed_time = None
                
                try:
                    p_even = subprocess.Popen(cmd_even, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
                    p_odd  = subprocess.Popen(cmd_odd, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
                    
                    start_race = time.time()
                    while time.time() - start_race < MAXTIME:
                        if p_even.poll() is not None:
                            race_stdout, _ = p_even.communicate()
                            break
                        if p_odd.poll() is not None:
                            race_stdout, _ = p_odd.communicate()
                            break
                        time.sleep(0.002) 
                        
                finally:
                    # Clean process handles
                    for proc in [p_even, p_odd]:
                        if proc and proc.poll() is None:
                            try: proc.terminate()
                            except: pass
                    # Force clean containers instantly via Docker Daemon
                    for c_name in [c_name_even, c_name_odd]:
                        subprocess.run(["docker", "kill", c_name], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                
                if race_stdout:
                    try: parsed_time = float(race_stdout.strip())
                    except ValueError: parsed_time = None

                # ------------------------------------------------------------
                # DUAL-TIMEOUT LOGICAL ENFORCEMENT & SHIELDING
                # ------------------------------------------------------------
                result_string = "TIMEOUT"
                if parsed_time is not None:
                    if parsed_time <= PAPER_TIMEOUT_THRESHOLD:
                        result_string = str(parsed_time)
                        time_sum += parsed_time
                        solved += 1
                        print(f" Done ({parsed_time}s)")
                    else:
                        time_sum += 120.0
                        print(f" LOGICAL TIMEOUT ({parsed_time}s reported)")
                else:
                    time_sum += 120.0
                    print(" PHYSICAL TIMEOUT / CRASHED")
                
                # LIVE LOGGING: Append data point directly to open row string
                with open(log_files[key], "a") as f:
                    f.write(f", {result_string}")

            # Close out log line for this domain block
            with open(log_files[key], "a") as f:
                f.write("\n")

            # DOMAIN COMPLETE: Calculate averages for summary sheet columns
            if solved > 0:
                avg_time = time_sum / solved
                time_str = f"{avg_time:.5f}"
            else:
                time_str = f"{time_sum:.5f}" if total > 0 else ""
            solved_str = f"{solved}/{total}" if total > 0 else ""
            
            summary_matrix_cache[algo['name']][key] = {'time': time_str, 'solved': solved_str}
            rewrite_summary_csv_live()

        except ImportError:
            print(f"  [Warning] Data module '{domain['module_name']}.py' missing! Skipping domain.", file=sys.stderr)
            with open(log_files[key], "a") as f:
                f.write(f"{algo['name']},ERROR_MISSING_CONFIG\n")
            summary_matrix_cache[algo['name']][key] = {'time': '', 'solved': ''}
            rewrite_summary_csv_live()