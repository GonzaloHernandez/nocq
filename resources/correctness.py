#!/usr/bin/env python3

import argparse
import subprocess
import sys
import time

n = 10
d = 5
NOCQ = "./nocq"

def run_command(args):
    """Execute a command, measure its duration, and return stdout and duration."""
    start_time = time.perf_counter()
    result = subprocess.run(
        args,
        capture_output=True,
        text=True,
        check=True
    )
    duration = time.perf_counter() - start_time
    return result.stdout.strip(), duration

def generate_instance():
    # We don't need to track the generation time for the solver comparison
    subprocess.run([
        NOCQ,
        "--rand", f"{n}", "5", "1", f"{d}",
        "--weights", "-10", "10",
        "--export-gmw", "rand.gm"
    ], capture_output=True, check=True)

def test_instance():
    ok = True
    total_normal_time = 0.0
    total_checker_time = 0.0

    for init in range(n):
        normal, t_normal = run_command([
            NOCQ,
            "--gm", "rand.gm",
            "--init", str(init),
            "--parity","--safety","3,4",
            "--prop-eager"
        ])
        total_normal_time += t_normal

        checker, t_checker = run_command([
            NOCQ,
            "--gm", "rand.gm",
            "--init", str(init),
            "--parity","--safety","3,4",
            "--prop-memo"
        ])
        total_checker_time += t_checker

        if normal != checker:
            print(f"\nMismatch for --init={init}")
            print(f"  eager : {normal}")
            print(f"  memo  : {checker}")
            ok = False

    # Calculate total and percentages for the current instance
    combined_time = total_normal_time + total_checker_time
    if combined_time > 0:
        pct_normal = (total_normal_time / combined_time) * 100
        pct_checker = (total_checker_time / combined_time) * 100
    else:
        pct_normal = pct_checker = 0.0

    # Print total time and the time distribution for this instance
    print(f" | Total: {combined_time:.3f}s (normal: {pct_normal:.1f}%, memo: {pct_checker:.1f}%)")

    return ok

def main():
    global n, d

    parser = argparse.ArgumentParser(description="Differential testing configuration.")
    parser.add_argument("n", type=int, nargs="?", default=10, help="Value for n (default: 10)")
    parser.add_argument("d", type=int, nargs="?", default=5, help="Value for d (default: 5)")
    args = parser.parse_args()

    # Expose variables for your generate_instance() or test_instance() functions
    n = args.n
    d = args.d

    iteration = 1
    print("Starting differential testing loop. Press Ctrl+C to stop.\n")
    
    try:
        while True:
            print(f"Running iteration {iteration}...", end="", flush=True)
            
            generate_instance()
            
            if not test_instance():
                print(f"\n[FAILURE] Broken rule found on iteration {iteration}!")
                print("The problematic instance has been preserved in 'rand.gm'.")
                sys.exit(1)
                
            iteration += 1
            
    except KeyboardInterrupt:
        print("\n\nTesting paused by user. No bugs found up to this point.")

if __name__ == "__main__":
    main()