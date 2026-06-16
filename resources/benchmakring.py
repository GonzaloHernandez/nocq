#!/usr/bin/env python3
import sys
import time
import threading
import docker
from requests.exceptions import ReadTimeout, ConnectionError

# ====================================================================
# IMPORT DATA AND INITIALIZE DOCKER CLIENT
# ====================================================================
try:
    from games_equivchecking import path, files, inits
except ImportError:
    print("[!] Error: Could not find 'benchmark_data.py' in this directory.", file=sys.stderr)
    sys.exit(1)

csv_file = "05_large_results_equivalncech.csv"
csv_log = "05_large_raw_equivalncech.csv"
maxtime = 70

try:
    client = docker.from_env()
except Exception as e:
    print(f"[!] Error: Could not connect to Docker Engine. Is Docker Desktop running?\n{e}", file=sys.stderr)
    sys.exit(1)

n = len(files)
volume_config = {path: {"bind": "/mnt", "mode": "rw"}}

solvers = {
    'PP':  {'count': 0, 'time': 0.0, 'image': 'solver', 'cmd': 'oink /mnt/{file} --pp'},
    'PP+': {'count': 0, 'time': 0.0, 'image': 'solver', 'cmd': 'oink /mnt/{file} --ppp'},
    'PAR': {'count': 0, 'time': 0.0, 'image': 'solver', 'cmd': 'oink /mnt/{file} --zlkpp-std'},
    'ZRA': {'count': 0, 'time': 0.0, 'image': 'nocq',   'cmd': '--gm /mnt/{file} --zra --print-only-time --init {init}'},
    'NOCQ':{'count': 0, 'time': 0.0}
}

# Clear and write headers to files
with open(csv_file, 'w') as f: f.write("time,solved\n")
with open(csv_log, 'w') as f: f.write("Oink(PP),Oink(PP+),Oink(PAR),ZRA,NOCQ\n")

print("-" * 48, file=sys.stderr)
print(f"Starting SDK Benchmarks... Saving summaries to {csv_file}", file=sys.stderr)
print("-" * 48, file=sys.stderr)

# ====================================================================
# RUNNER HELPERS
# ====================================================================
def run_container_safely(image, command):
    """Starts a container in background, waits with a timeout, and returns stdout."""
    container = None
    try:
        # Run detached container to allow manual timeout monitoring
        container = client.containers.run(image=image, command=command, volumes=volume_config, detach=True)
        
        # Enforce execution timeout via the container engine wrapper socket
        result = container.wait(timeout=maxtime)
        
        stdout = container.logs().decode("utf-8")
        return stdout
    except (ReadTimeout, ConnectionError, docker.errors.ContainerError):
        return "TIMEOUT"
    finally:
        if container:
            try:
                container.remove(force=True)
            except:
                pass

def race_worker(image, command, result_dict, key):
    """Target function for background threads during the parallel race phase."""
    stdout = run_container_safely(image, command)
    clean_out = stdout.strip() if stdout != "TIMEOUT" else "TIMEOUT"
    
    # Verify string outcome contains digits/floats instead of error text
    if clean_out != "TIMEOUT" and clean_out.replace('.', '', 1).isdigit():
        result_dict[key] = clean_out
    else:
        result_dict[key] = "TIMEOUT"

# ====================================================================
# BENCHMARK ENGINE LOOP
# ====================================================================
try:
    for i in range(n):
        file = files[i]
        init_val = str(inits[i])
        current_row = {}
        
        print(f"Processing game {i+1}/{n}: {file} ...", file=sys.stderr)
        print("Oink(PP)... Oink(PP+)... Oink(PAR)... ZRA... NOCQ...", file=sys.stderr)

        # 1. Evaluate Sequential Solvers (Oink and ZRA)
        for label in ['PP', 'PP+', 'PAR', 'ZRA']:
            cfg = solvers[label]
            formatted_cmd = cfg['cmd'].format(file=file, init=init_val)
            
            stdout = run_container_safely(cfg['image'], formatted_cmd)
            
            parsed_time = "TIMEOUT"
            if stdout != "TIMEOUT":
                if label == 'ZRA':
                    clean_zra = stdout.strip()
                    if clean_zra.replace('.', '', 1).isdigit():
                        parsed_time = clean_zra
                else: # Oink structural string parsing
                    for line in stdout.splitlines():
                        if "total_solving_time:" in line.replace(" ", "_"):  # Safeguard spacing variations
                            parts = line.split()
                            # Find where 'sec.' or 'sec' is, and take the element right before it
                            if "sec." in parts or "sec" in parts:
                                idx = parts.index("sec.") if "sec." in parts else parts.index("sec")
                                raw_val = parts[idx - 1]
                            else:
                                raw_val = parts[-2]  # Fallback to second-to-last item
                            
                            # Strip away any accidental trailing punctuation just in case
                            raw_val = raw_val.strip(",")
                            
                            # Validate and convert to float
                            try:
                                float(raw_val)
                                parsed_time = raw_val
                            except ValueError:
                                pass  # Not a valid number, keep parsed_time as "TIMEOUT"
                            break
            
            current_row[label] = parsed_time
            if parsed_time != "TIMEOUT":
                cfg['count'] += 1
                cfg['time'] += float(parsed_time)
                # print(f"   --> {label}: {parsed_time}s", file=sys.stderr)
            else:
                cfg['time'] += 120.0
                print(f"   --> Warning: {label} failed or timed out!", file=sys.stderr)

        # ====================================================================
        # 2. Evaluate NOCQ Parallel Race (EVEN vs ODD) - With Crash Diagnosis
        # ====================================================================
        cmd_even = f"--gm /mnt/{file} --noc-even --parity --print-only-totaltime --init {init_val}"
        cmd_odd  = f"--gm /mnt/{file} --noc-odd --parity --print-only-totaltime --init {init_val}"
        
        c_even = None
        c_odd = None
        nocq_final = "TIMEOUT"
        
        try:
            # Fire off both containers completely detached
            c_even = client.containers.run(image="nocq", command=cmd_even, volumes=volume_config, detach=True)
            c_odd  = client.containers.run(image="nocq", command=cmd_odd, volumes=volume_config, detach=True)
            
            start_race = time.time()
            while time.time() - start_race < maxtime:
                try: c_even.reload()
                except: pass
                try: c_odd.reload()
                except: pass
                
                # Check EVEN
                if c_even.status == "exited":
                    logs = c_even.logs().decode("utf-8").strip()
                    # Validate that the output can be interpreted as a valid float
                    try:
                        float(logs)
                        nocq_final = logs
                    except ValueError:
                        print(f"\n[!] EVEN crashed instantly! Docker logs:\n{logs}", file=sys.stderr)
                    break
                    
                # Check ODD
                if c_odd.status == "exited":
                    logs = c_odd.logs().decode("utf-8").strip()
                    try:
                        float(logs)
                        nocq_final = logs
                    except ValueError:
                        print(f"\n[!] ODD crashed instantly! Docker logs:\n{logs}", file=sys.stderr)
                    break
                
                time.sleep(0.005)
                
        finally:
            for container in [c_even, c_odd]:
                if container:
                    try:
                        container.remove(force=True)
                    except:
                        pass

        current_row['NOCQ'] = nocq_final
        if nocq_final != "TIMEOUT":
            solvers['NOCQ']['count'] += 1
            solvers['NOCQ']['time'] += float(nocq_final)
            print(f"   --> NOCQ: {nocq_final}s", file=sys.stderr)
        else:
            solvers['NOCQ']['time'] += 120.0
            print("   --> Warning: NOCQ failed or timed out!", file=sys.stderr)

        # Append step metrics directly to file logs
        with open(csv_log, 'a') as f:
            f.write(f"{current_row['PP']},{current_row['PP+']},{current_row['PAR']},{current_row['ZRA']},{current_row['NOCQ']}\n")

except KeyboardInterrupt:
    print("\n[!] Loop aborted via user interrupt. Compiling partial summaries...", file=sys.stderr)

# ====================================================================
# COMPILE AND LOG ACCUMULATED RUN SUMMARY METRICS
# ====================================================================
print("-" * 48, file=sys.stderr)
print(f"Done! Summaries written to {csv_file}", file=sys.stderr)
print("-" * 48, file=sys.stderr)

labels_mapping = [('Oink(PP)', 'PP'), ('Oink(PP+)', 'PP+'), ('Oink(PAR)', 'PAR'), ('ZRA', 'ZRA'), ('NOCQ', 'NOCQ')]
with open(csv_file, 'a') as f:
    for name, key in labels_mapping:
        metrics = solvers[key]
        if metrics['count'] > 0:
            avg = metrics['time'] / metrics['count']
            f.write(f"{name},{avg:.5f},{metrics['count']}/{n}\n")
        else:
            f.write(f"{name},{n * 120}.00000,{metrics['count']}/{n}\n")