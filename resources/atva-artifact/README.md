# Artifact Evaluation: NOCQ
This artifact provides the complete toolchain, source code, and instructions required to run **NOCQ** to reproduce and complement the results described in our accompanying paper.

---

## 1. Description of the Artifact Content

### Purpose
The primary purpose of this artifact is to allow reviewers to independently verify, compile, and run `nocq`. It packages all necessary dependencies, compilers, and libraries within a self-contained environment. 

Additionally, we provide a complete benchmark suite, including a copy of the public PGSolver dataset [1] and other benchmarks generated following the methodologies in [1], to test the tool and reproduce our experiments. 

> ⚠️ **Important Note on Replication & Disk Space:**
> * **Variability:** Exact execution times and values may vary slightly from the paper due to random seeds used during experiment runs.
> * **Storage Requirement:** Please ensure you have at least **70 GB** of free disk space available after unzipping the benchmark datasets.
> * **SAT Solvers & DIMACS Overhead:** If you choose to run the external SAT solvers (CaDiCaL, Kissat, or MiniSat), additional disk space will be required during runtime to temporarily generate and store the translated DIMACS (.cnf) files for the games.

### Paper Claims Supported
This artifact implements an open-source tool designed to solve parity games, energy games, and games with mean-payoff conditions. While this specific artifact package is a scaled-down version not intended to natively regenerate every massive table in the paper, it contains the exact toolchain and execution scripts used to collect our data. We provide evaluation scripts to demonstrate how this raw output is generated before post-processing.

* **Claim 1 (Performance / Correctness):** `nocq` successfully processes the evaluated benchmarks without errors, demonstrating the correctness and viability of the underlying Constraint Satisfaction Framework.

### Structure of the Artifact
The artifact consists of the following components:

* Core Toolchain & Documentation Bundle (`nocq-atva-artifact.zip`): A single compressed archive enclosing the primary environment and setup files: `solver-image.tar.gz`: Pre-built and self-contained, enclosing the full execution environment, NOCQ, and baseline solvers (Oink, Cadical, Kissat, MiniSat), `LICENSE` File: Outlining the open-source licensing terms governing this software framework, and README File `README.md`: This instruction and configuration guide.

* Benchmark Sets: 5 Zip archives containing game instances categorized by evaluation type (`equivchecking`, `modelchecking`, `pgsolver`, `random`, and `sprand`).

* Package of python scripts (`python_scripts.zip`): 4 Main Python Scripts: Execution entry points (`benchmarking_t*.py`) configured to reconstruct Tables 1–4 from the accompanying paper, and 8 Auxiliary Python Scripts: Configuration files (`games_*.py`) that manage and map specific game suites to the main evaluation runs.


## 2. Installation Instructions
Prerequisites
* Docker installed and running on your host system.
* Python 3 installed on your host system (with the `docker` library).

### Step 1: Load the Docker Image
Load the pre-built image archive into your local Docker environment:
```
docker load < solver-image.tar.gz
```

### Step 2: Smoke Test
Verify the installation using the following two quick tests:

#### Test 1: Verify Help Output
Running `nocq` without parameters should display the command-line help interface
```
docker run --rm --platform linux/amd64 -it solver nocq
```
Expected Output
```
NOCQ: A Constraint-Based Toolchain for Parity Games with Quantitative Conditions.
Usage: ./build/nocq [options]

Game creation:
  --dzn <filename>           : Load DZN file
  --gm <filename>            : Load GM file
  --jurd <levels> <blocks>   : Jurdzinski game
  --rand <ns> <ps> <d1> <d2> : Random game
  --mladder <bl>             : ModelcheckerLadder game
  ...
```

#### Test 2: Generate and Solve a Synthesized Game
Generate a Jurdzinski game with 3 levels and 2 blocks, and solve it under parity conditions:

```
docker run --rm --platform linux/amd64 -it solver nocq --jurd 3 2 --parity --print-times
```
Expected Output
```
Game creation time : 8.2766e-05
Init time          : 0.000147634
Solving time       : 0.000232157
Result             : EVEN
```

## 3. Running the Benchmarks
### Step 1: Extract the Datasets and Python Scripts
Before running the evaluation scripts, ensure that the 5 benchmark sets are completely unzipped into a folder named `games/` within the root directory.

Your local directory tree must look like this:
```
nocq-atva-artifact/
│
├── games/
│   ├── equivchecking/
│   ├── modelchecking/
│   ├── pgsolver/
│   ├── random/
│   └── sprand/
```
Unzip the python scripts directly in the root folder (12 files)

```
...
├── benchmarking_t1.py
├── benchmarking_t2.py
...
├── games_equivchecking.py
├── games_modelchecking.py
...

```

### Step 2: Install Python Dependencies
The host execution scripts require the Python `docker` SDK to communicate with the container backend:
```
pip install docker
```
### Step 3: Run Evaluation Scripts
We provide 4 main Python scripts to reconstruct the corresponding 4 tables presented in the paper. Execute them directly via your terminal:
```
python3 benchmarking_t1.py
python3 benchmarking_t2.py
python3 benchmarking_t3.py
python3 benchmarking_t4.py
```
### Output Files
Each script automatically executes the required games, benchmarks them against the Docker image, and generates primary summary CSV files matching the layout of the tables in the paper:
```
t1_small_summary.csv
t2_large_summary.csv
t3_all_summary.csv
t4_sprand_summary.csv
```
ℹ️ Note: Additional generated CSV files contain fine-grained execution timestamps per game. These can be used for deep-dive tracking or safely ignored.

> ⚠️ **Important Configuration for Replication:**
> * **External Solvers:** To speed up replication, you can comment out rows for solvers that you do not wish to evaluate. In `benchmarking_t1.py` and `benchmarking_t2.py`, these can be configured around lines 124 to 131. For example, to only run NOCQ:
```Python
algorithms_pool = [
    # {'name': 'Oink(PP)',    'type': 'oink',         'flag': '--pp'},
    # {'name': 'Oink(PP+)',   'type': 'oink',         'flag': '--ppp'},
    # {'name': 'Oink(PAR)',   'type': 'oink',         'flag': '--zlkpp-std'},
    # {'name': 'ZRA(ImpAttr)','type': 'zra',          'flag': '--zra'},
      {'name': 'nocq(Ours)',  'type': 'nocq_race',    'flag': None},
    # {'name': 'CaDiCaL',     'type': 'sat_pipeline', 'flag': 'cadical'}, 
    # {'name': 'Kissat',      'type': 'sat_pipeline', 'flag': 'kissat'}, 
    # {'name': 'Minisat',     'type': 'sat_pipeline', 'flag': 'minisat'}
  ]
``` 
> * **Tables 3 and 4**: The similar adjustments can be made in benchmarking_t3.py and benchmarking_t4.py around lines 56 to 62.

## Additional Customization

For a complete list of runtime parameters—including 6 different output formats—please refer to the `--help` flag documentation.

Extended configuration details, advanced usage examples, and the latest updates can also be found in our GitHub Repository Readme https://github.com/gonzalohernandez/nocq.

## References
[1] Keiren, J.J.A.: Benchmarks for parity games (extended version) (2015). Available at: https://arxiv.org/abs/1407.3121
