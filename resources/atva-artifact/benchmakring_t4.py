#!/usr/bin/env python3
import subprocess
import sys
import os
import time

# ====================================================================
# TIMEOUT CONFIGURATION
# ====================================================================
PAPER_TIMEOUT_THRESHOLD = 60.0
MAXTIME = 120  # Explicit 120s execution wall-clock ceiling

# Table 4 specific files layout
summary_csv = "t4_sprand_summary.csv"
log_files = {
    'sprand_b':  "t4_sprand_b.csv",
    'sprand_h':  "t4_sprand_h.csv",
    'sprand_xb': "t4_sprand_xb.csv",
    'sprand_xh': "t4_sprand_xh.csv"
}

# Domains mapped directly to your 4 benchmark module sheets
domains_setup = [
    {'key': 'sprand_b',  'module_name': 'games_sprand_b',  'desc': 'R_10b'},
    {'key': 'sprand_h',  'module_name': 'games_sprand_h',  'desc': 'R_10h'},
    {'key': 'sprand_xb', 'module_name': 'games_sprand_xb', 'desc': 'R_10xb'},
    {'key': 'sprand_xh', 'module_name': 'games_sprand_xh', 'desc': 'R_10xh'}
]

summary_matrix_cache = {}

def rewrite_summary_csv_live():
    """Rewrites the central Table 4 summary matrix sheet dynamically."""
    with open(summary_csv, "w") as f:
        f.write("Conditions,R_10b_Time,R_10b_Solved,R_10h_Time,R_10h_Solved,R_10xb_Time,R_10xb_Solved,R_10xh_Time,R_10xh_Solved\n")
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
# ALGORITHMS CONFIGURATION SUITE (TABLE 4 ROWS)
# ====================================================================
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
    print(f"Evaluating Table 4 Row: {algo['name']} Across SPRAND Families")
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
            
            # Read standardized arrays from the module directly
            files_list = getattr(g_module, 'files', [])
            inits_list = getattr(g_module, 'inits', [])
            energy_list = getattr(g_module, 'thenergy', [])
            mean_list = getattr(g_module, 'thmean', [])
            
            total = len(files_list)
            
            if total == 0:
                print(f"  [Info] No files loaded for {domain['module_name']}. Skipping domain.")
                summary_matrix_cache[algo['name']][key] = {'time': '', 'solved': ''}
                rewrite_summary_csv_live()
                continue

            # LIVE LOGGING: Write the condition row header
            with open(log_files[key], "a") as f:
                f.write(f"{algo['name']}")

            for idx, filename in enumerate(files_list):
                print(f"  [{idx+1}/{total}] Running {filename}...", end="", flush=True)
                
                # Fetch explicit weights and parameters securely
                init_val = str(inits_list[idx]) if idx < len(inits_list) else "0"
                th_energy = str(energy_list[idx]) if idx < len(energy_list) else "0"
                th_mean = str(mean_list[idx]) if idx < len(mean_list) else "0"

                # Build runtime flags for the solver
                flag_args = ["--chuffed", "--init", init_val, "--print-only-time"]
                
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
                c_name_even = f"t4_race_even_{idx}"
                c_name_odd  = f"t4_race_odd_{idx}"

                cmd_even = ["docker", "run", "--rm", "--platform", "linux/amd64", "--name", c_name_even, "--init", "-v", f"{base_path}:/mnt", "solver", 
                            "nocq", "--dzn", f"/mnt/{filename}", "--noc-even"] + flag_args
                cmd_odd  = ["docker", "run", "--rm", "--platform", "linux/amd64", "--name", c_name_odd, "--init", "-v", f"{base_path}:/mnt", "solver", 
                            "nocq", "--dzn", f"/mnt/{filename}", "--noc-odd"] + flag_args
                
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
                    # Clear local hooks
                    for proc in [p_even, p_odd]:
                        if proc and proc.poll() is None:
                            try: proc.terminate()
                            except: pass
                    # Kill slow containers instantly via daemon handles
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
                        print(f" Done")
                    else:
                        time_sum += 120.0
                        print(f" LOGICAL TIMEOUT")
                else:
                    time_sum += 120.0
                    print(" PHYSICAL TIMEOUT")
                
                # LIVE LOGGING: Append data point instantly to raw log files
                with open(log_files[key], "a") as f:
                    f.write(f", {result_string}")

            # Close out row lines cleanly
            with open(log_files[key], "a") as f:
                f.write("\n")

            # DOMAIN COMPLETE: Calculate column summary data metrics
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