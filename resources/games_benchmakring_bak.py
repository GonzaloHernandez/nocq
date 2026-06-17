#!/usr/bin/env python3
import subprocess
import sys
import os

# ====================================================================
# TIMEOUT CONFIGURATION
# ====================================================================
MAXTIME = 120  
PAPER_TIMEOUT_THRESHOLD = 60.0

# Setup Global Out File Structures
summary_csv = "t1_summary.csv"
log_files = {
    'equiv':  "t1_equivchecking.csv",
    'model':  "t1_modelchecking.csv",
    'pgsol':  "t1_pgsolver.csv",
    'random': "t1_random.csv"
}

domains_setup = [
    {'key': 'equiv',  'module_name': 'games_equivchecking',  'desc': 'Equivalence Checking'},
    {'key': 'model',  'module_name': 'games_modelchecking',  'desc': 'Model Checking'},
    {'key': 'pgsol',  'module_name': 'games_pgsolver',       'desc': 'PGSolver'},
    {'key': 'random', 'module_name': 'games_random',         'desc': 'RandomG'}
]

# Cache to hold ONLY completed data blocks in memory
summary_matrix_cache = {}

def rewrite_summary_csv_live():
    """Rewrites the summary CSV showing ONLY data that has actually been computed."""
    with open(summary_csv, "w") as f:
        f.write("Algorithm,EquivcheckingTime,EquivcheckingSolved,ModelcheckingTime,ModelcheckingSolved,PGSolverTime,PGSolverSolved,RandomGTime,RandomGSolved\n")
        for algo_name, domain_data in summary_matrix_cache.items():
            row = [algo_name]
            for domain in domains_setup:
                stats = domain_data[domain['key']]
                if stats:  # If this domain has finished, append its metrics
                    row.extend([stats['time'], stats['solved']])
                else:      # Otherwise, leave these columns empty
                    row.extend(["", ""])
            # Strip trailing commas from the right if the row ends early
            f.write(",".join(row).rstrip(",") + "\n")

# Initialize Log Files cleanly if they don't exist yet
for path_log in log_files.values():
    if not os.path.exists(path_log):
        with open(path_log, "w") as f:
            pass 

# ====================================================================
# UTILITY EXECUTION PIPELINE
# ====================================================================
def run_solver_instance(base_path, image, command_args):
    """Executes standard CLI process tracking. Returns raw stdout or TIMEOUT."""
    cmd = ["docker", "run", "--rm", "-t", "--init", "-v", f"{base_path}:/mnt", image] + command_args
    try:
        res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=MAXTIME)
        return res.stdout
    except subprocess.TimeoutExpired:
        return "TIMEOUT"

def parse_oink_time(stdout_text):
    """Parses Oink internal execution prints safely regardless of text alignment."""
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
            
            raw_val = raw_val.strip(",")
            try:
                return float(raw_val)
            except ValueError:
                return None
    return None

# ====================================================================
# ALGORITHMS CONFIGURATION SUITE
# ====================================================================
algorithms_pool = [
    {'name': 'Oink(PP)',      'type': 'oink', 'flag': '--pp'},
    # {'name': 'Oink(PP+)',     'type': 'oink', 'flag': '--ppp'},
    # {'name': 'Oink(PAR)',     'type': 'oink', 'flag': '--zlkpp-std'},
    {'name': 'ZRA(ImpAttr)',  'type': 'zra',  'flag': '--zra'}
]

# ====================================================================
# MAIN EVALUATION RUNNER ENGINE
# ====================================================================
for algo in algorithms_pool:
    print("=" * 75)
    print(f"Evaluating Algorithm: {algo['name']} Across All Domains")
    print("=" * 75)
    
    # Initialize entries as None (completely empty) instead of fake zeros
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
            files_list = g_module.files_small
            inits_list = getattr(g_module, 'inits_small', [])
            
            total = len(files_list)
            
            if total == 0:
                print(f"  [Warning] No small files defined in {domain['module_name']}.py")
                with open(log_files[key], "a") as f:
                    f.write(f"{algo['name']},NO_GAMES\n")
                summary_matrix_cache[algo['name']][key] = {'time': '', 'solved': ''}
                rewrite_summary_csv_live()
                continue

            # LIVE LOGGING: Write the algorithm prefix at the start of the raw log row
            with open(log_files[key], "a") as f:
                f.write(f"{algo['name']}")

            for idx, filename in enumerate(files_list):
                print(f"  [{idx+1}/{total}] Running {filename}...", end="", flush=True)
                
                if algo['type'] == 'oink':
                    args = ["oink", f"/mnt/{filename}", algo['flag']]
                else:
                    init_val = str(inits_list[idx]) if idx < len(inits_list) else "0"
                    args = ["nocq", "--gm", f"/mnt/{filename}", algo['flag'], "--print-only-time", "--init", init_val]

                raw_output = run_solver_instance(base_path, "solver", args)
                
                parsed_time = None
                if raw_output != "TIMEOUT" and raw_output:
                    if algo['type'] == 'oink':
                        parsed_time = parse_oink_time(raw_output)
                    else:
                        try:
                            parsed_time = float(raw_output.strip())
                        except ValueError:
                            parsed_time = None

                # --- DUAL-TIMEOUT LOGICAL ENFORCEMENT ---
                result_string = "TIMEOUT"
                if parsed_time is not None:
                    if parsed_time <= PAPER_TIMEOUT_THRESHOLD:
                        result_string = str(parsed_time)
                        time_sum += parsed_time
                        solved += 1
                        # print(f" Done ({parsed_time}s)")
                        print(" Done")
                    else:
                        time_sum += 120.0
                        # print(f" LOGICAL TIMEOUT ({parsed_time}s reported, > {PAPER_TIMEOUT_THRESHOLD}s cutoff)")
                        print(" Timeout")
                else:
                    time_sum += 120.0
                    # print(" PHYSICAL TIMEOUT / CRASHED")
                    print(" Timeout")
                
                # LIVE LOGGING: Append to raw log rows instantly
                with open(log_files[key], "a") as f:
                    f.write(f", {result_string}")

            # LIVE LOGGING: Close the log row for this domain
            with open(log_files[key], "a") as f:
                f.write("\n")

            # DOMAIN COMPLETE: Compute real averages for this block
            if solved > 0:
                avg_time = time_sum / solved
                time_str = f"{avg_time:.5f}"
            else:
                time_str = f"{time_sum:.5f}" if total > 0 else ""
            solved_str = f"{solved}/{total}" if total > 0 else ""
            
            # Save the valid block stats and overwrite the live summary file cleanly
            summary_matrix_cache[algo['name']][key] = {'time': time_str, 'solved': solved_str}
            rewrite_summary_csv_live()

        except ImportError:
            print(f"  [Warning] Data module '{domain['module_name']}.py' missing! Skipping domain.", file=sys.stderr)
            with open(log_files[key], "a") as f:
                f.write(f"{algo['name']},ERROR_MISSING_CONFIG\n")
            summary_matrix_cache[algo['name']][key] = {'time': '', 'solved': ''}
            rewrite_summary_csv_live()