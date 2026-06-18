#!/usr/bin/env python3
import subprocess
import sys
import os
import time

# ====================================================================
# MASTER SELECTION PARAMETER
# ====================================================================
TABLE_MODE = "Table2" 

# ====================================================================
# AUTOMATIC PROFILE ROUTING CONFIGURATION
# ====================================================================
PAPER_TIMEOUT_THRESHOLD = 60.0
MAXTIME = 120  # Unified 120s execution ceiling for all benchmarks

if TABLE_MODE == "Table1":
    file_prefix = "t1_small_"
    target_file_array = "files_small"
    target_init_array = "inits_small"
    print(">>> PROFILE CONFIGURATION: Booting Table 1 Engine (Small Games) <<<", file=sys.stderr)
elif TABLE_MODE == "Table2":
    file_prefix = "t2_large_"
    target_file_array = "files_large"
    target_init_array = "inits_large"
    print(">>> PROFILE CONFIGURATION: Booting Table 2 Engine (Large Games) <<<", file=sys.stderr)
else:
    print(f"[!] Critical Error: Unknown TABLE_MODE '{TABLE_MODE}'.", file=sys.stderr)
    sys.exit(1)

summary_csv = f"{file_prefix}summary.csv"
log_files = {
    'equiv':  f"{file_prefix}equivchecking.csv",
    'model':  f"{file_prefix}modelchecking.csv",
    'pgsol':  f"{file_prefix}pgsolver.csv",
    'random': f"{file_prefix}random.csv"
}

domains_setup = [
    {'key': 'equiv',  'module_name': 'games_equivchecking',  'desc': 'Equivalence Checking'},
    {'key': 'model',  'module_name': 'games_modelchecking',  'desc': 'Model Checking'},
    {'key': 'pgsol',  'module_name': 'games_pgsolver',       'desc': 'PGSolver'},
    {'key': 'random', 'module_name': 'games_random',         'desc': 'RandomG'}
]

summary_matrix_cache = {}

def rewrite_summary_csv_live():
    """Rewrites the summary CSV showing ONLY data that has actually been computed."""
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
# UTILITY PARSING FUNCTIONS
# ====================================================================
def run_solver_instance(base_path, image, command_args):
    """Executes standard CLI process tracking. Returns raw stdout or TIMEOUT."""
    cmd = ["docker", "run", "--rm", "--platform", "linux/amd64", "-t", "--init", "-v", f"{base_path}:/mnt", image] + command_args
    try:
        res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=MAXTIME)
        return res.stdout
    except subprocess.TimeoutExpired:
        return "TIMEOUT"

def parse_oink_time(stdout_text):
    """Parses Oink internal execution prints safely."""
    if stdout_text == "TIMEOUT" or not stdout_text:
        return None
    for line in stdout_text.splitlines():
        if "total_solving_time:" in line.replace(" ", "_"):
            parts = line.split()
            if "sec." in parts or "sec" in parts:
                idx = parts.index("sec.") if "sec." in parts else parts.index("sec")
                raw_val = parts[idx - 1]
            else:
                raw_val = parts[-2]
            return float(raw_val.strip(","))
    return None

def parse_sat_solver_time(stdout_text, solver_type):
    """Parses execution time from CaDiCaL, Kissat, or Minisat stdout match matrices."""
    if stdout_text == "TIMEOUT" or not stdout_text:
        return None
    
    for line in stdout_text.splitlines():
        if solver_type == "cadical" and "total process time since initialization" in line:
            parts = line.split()
            try: return float(parts[-2])
            except (ValueError, IndexError): return None
            
        elif solver_type == "kissat" and "process-time:" in line:
            parts = line.split()
            try: return float(parts[-2])
            except (ValueError, IndexError): return None
            
        elif solver_type == "minisat" and "CPU time" in line:
            parts = line.split()
            try: return float(parts[-2])
            except (ValueError, IndexError): return None
            
    return None

# ====================================================================
# ALGORITHMS CONFIGURATION SUITE
# ====================================================================
# Comment out any rows you aren't currently testing to focus execution
algorithms_pool = [
    {'name': 'Oink(PP)',      'type': 'oink',        'flag': '--pp'},
    {'name': 'Oink(PP+)',     'type': 'oink',        'flag': '--ppp'},
    {'name': 'Oink(PAR)',     'type': 'oink',        'flag': '--zlkpp-std'},
    {'name': 'ZRA(ImpAttr)',  'type': 'zra',         'flag': '--zra'},
    {'name': 'nocq(Ours)',    'type': 'nocq_race',   'flag': None},
    {'name': 'CaDiCaL',       'type': 'sat_pipeline', 'flag': 'cadical'}, 
    {'name': 'Kissat',        'type': 'sat_pipeline', 'flag': 'kissat'}, 
    {'name': 'Minisat',       'type': 'sat_pipeline', 'flag': 'minisat'}
]

# ====================================================================
# MAIN EVALUATION RUNNER ENGINE
# ====================================================================
for algo in algorithms_pool:
    print("=" * 75)
    print(f"Evaluating Algorithm: {algo['name']} Across All Domains [{TABLE_MODE}]")
    print("=" * 75)
    
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
            
            files_list = getattr(g_module, target_file_array, [])
            inits_list = getattr(g_module, target_init_array, [])
            
            total = len(files_list)
            
            if total == 0:
                print(f"  [Info] No games defined for configuration array '{target_file_array}'. Skipping.")
                summary_matrix_cache[algo['name']][key] = {'time': '', 'solved': ''}
                rewrite_summary_csv_live()
                continue

            # LIVE LOGGING: Write the algorithm prefix at the start of the log row
            with open(log_files[key], "a") as f:
                f.write(f"{algo['name']}")

            for idx, filename in enumerate(files_list):
                print(f"  [{idx+1}/{total}] Running {filename}...", end="", flush=True)
                init_val = str(inits_list[idx]) if idx < len(inits_list) else "0"

                parsed_time = None

                # ------------------------------------------------------------
                # PATH A: SEQUENTIAL RUNNERS (Oink & ZRA)
                # ------------------------------------------------------------
                if algo['type'] in ['oink', 'zra']:
                    if algo['type'] == 'oink':
                        args = ["oink", f"/mnt/{filename}", algo['flag']]
                    else:
                        args = ["nocq", "--gm", f"/mnt/{filename}", algo['flag'], "--print-only-time", "--init", init_val]

                    raw_output = run_solver_instance(base_path, "solver", args)
                    
                    if raw_output != "TIMEOUT" and raw_output:
                        if algo['type'] == 'oink':
                            parsed_time = parse_oink_time(raw_output)
                        else:
                            try: parsed_time = float(raw_output.strip())
                            except ValueError: parsed_time = None

                # ------------------------------------------------------------
                # PATH B: ASYNCHRONOUS PARALLEL RACE ENGINE (nocq Ours)
                # ------------------------------------------------------------
                elif algo['type'] == 'nocq_race':
                    c_name_even = f"nocq_race_even_{idx}"
                    c_name_odd  = f"nocq_race_odd_{idx}"

                    cmd_even = ["docker", "run", "--rm", "--platform", "linux/amd64", "--name", c_name_even, "--init", "-v", f"{base_path}:/mnt", "solver", 
                                "nocq", "--gm", f"/mnt/{filename}", "--noc-even", "--parity", "--print-only-totaltime", "--init", init_val]
                    cmd_odd = ["docker", "run", "--rm", "--platform", "linux/amd64", "--name", c_name_odd, "--init", "-v", f"{base_path}:/mnt", "solver", 
                               "nocq", "--gm", f"/mnt/{filename}", "--noc-odd", "--parity", "--print-only-totaltime", "--init", init_val]
                    
                    p_even = None
                    p_odd = None
                    race_stdout = None
                    
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
                        for proc in [p_even, p_odd]:
                            if proc and proc.poll() is None:
                                try: proc.terminate()
                                except: pass
                        for c_name in [c_name_even, c_name_odd]:
                            subprocess.run(["docker", "kill", c_name], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                    
                    if race_stdout:
                        try: parsed_time = float(race_stdout.strip())
                        except ValueError: parsed_time = None

                # ------------------------------------------------------------
                # PATH C: TWO-STEP UNIFIED SAT SOLVER PIPELINE (CaDiCaL, Kissat, Minisat)
                # ------------------------------------------------------------
                elif algo['type'] == 'sat_pipeline':
                    dimacs_filename = f"{filename}_{idx}.cnf"
                    
                    # Step 1: Same dynamic DIMACS CNF compilation for all solvers via nocq
                    encode_args = ["nocq", "--gm", f"/mnt/{filename}", "--init", init_val, "--sat-encoding", f"/mnt/{dimacs_filename}"]
                    encode_output = run_solver_instance(base_path, "solver", encode_args)
                    
                    if encode_output == "TIMEOUT":
                        parsed_time = None
                    else:
                        # Step 2: Dynamically route command based on backend name flag
                        solve_args = [algo['flag'], f"/mnt/{dimacs_filename}"]
                        raw_output = run_solver_instance(base_path, "solver", solve_args)
                        parsed_time = parse_sat_solver_time(raw_output, algo['flag'])
                    
                    # Disk Clean-up: Purge intermediate heavy .cnf asset instantly
                    local_cnf_path = os.path.join(base_path, dimacs_filename)
                    if os.path.exists(local_cnf_path):
                        try: os.remove(local_cnf_path)
                        except: pass

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
                
                # LIVE LOGGING: Append data point
                with open(log_files[key], "a") as f:
                    f.write(f", {result_string}")

            with open(log_files[key], "a") as f:
                f.write("\n")

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