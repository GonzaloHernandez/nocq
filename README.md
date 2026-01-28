# NOCQ: A Quantitative Parity Game Solver

NOCQ is a high-performance C++ tool designed for solving parity games with quantitative conditions. It combines classic graph-based algorithms with modern constraint programming techniques by integrating the **Chuffed** solver.

This tool is submitted as part of the **CAV 2026** tool paper track.

## Project Structure

* `src/main.cpp`:   Main entry point and CLI logic.
* `src/cp_nocq/`:   CP Models and Propagators
* `src/utils/`:     Core implementations:
* `thirdparty/`:    External dependencies (includes a local copy of the Chuffed solver).
* `resources/`:     Script for solve in parallel by fliping the game:
* `examples/`:      DZN and GMW files as example. 
                    Include AUD files to edit arenas 
                    [github.com/gonzalohernandez/graphing](https://github.com/GonzaloHernandez/graphing):

## Prerequisites

To build NOCQ, you need a Linux environment (tested on Arch Linux) with:
* **CMake** (v3.10 or higher)
* **GCC/G++** (v11 or higher, supporting C++17)
* **Bison** and **Flex** (required for building Chuffed)
* **Pthreads** library

## Installation & Building

NOCQ uses a CMake Superbuild system that automatically handles the compilation of the Chuffed solver.

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Usage

Run NOCQ using the following syntax:

```bash
./nocq [options] <arguments>
```
### Input & Game Generation

NOCQ can solve games from files or generate standard benchmarks on the fly:

**From File:**
* `--dzn <filename>`: Load a game from a MiniZinc data file (.dzn).
* `--gm <filename>`: Load a game from a PGSolver format file (.gm/.gmw).

**Benchmark Generators:**
* `--jurd <l> <b>`: Generate a Jurdzinski game with $l$ levels and $b$ blocks.
* `--rand <n> <p> <d1> <d2>`: Generate a random parity game with $n$ vertices, a maximum priority $p$, and outgoing degrees between $d_1$ and $d_2$.
* `--mladder <b>`: Generate a Modelchecker Ladder game with $b$ blocks.

### Solving Engines & Conditions

**Solvers:**
* `--chuffed`: Use the Chuffed CP solver (default).
* `--gecode`: Use the Gecode CP solver (if enabled).
* `--noc-even` / `--noc-odd`: Solve for a specific player (EVEN or ODD) using the NOC approach.
* `--fra`: Solve using the Fordward Recursive Algorithm.
* `--zra`: Solve using Zielonka's Recursive Algorithm.
* `--sat-encoding <filename>`: Encode the game into DIMACS format and save it to a file.
* `--scc`: Decompose the game graph into Strongly Connected Components (SCCs) to optimize solving.

**Conditions:**
* `--parity`: Parity condition (default).
* `--energy`: Energy condition.
* `--mean-payoff`: Mean-Payoff condition.
* `--threshold <t>`: Set the threshold value $t$ for Mean-Payoff games.

### Output & Verbosity

**Formatting:**
* `--verbose`: Print full execution details and progress.
* `--print-game`: Print the game graph structure.
* `--print-solution`: Print the winning regions and strategies for all vertices.
* `--print-statistics`: Print performance metrics after solving.

**Timing:**
* `--print-time`: Print only the solving time.
* `--print-times`: Print all timing data.

### Transformation & Export

**Weights:**
* `--weights <w1> <w2>`: Define the range for edge weights from $w_1$ to $w_2$.
* `--weights-force <w1> <w2>`: Force the use of the specified weight range $w_1$ to $w_2$.

**Exporting:**
* `--export-dzn <filename>`: Save the current game in DZN format.
* `--export-gm <filename>`: Save the current game in GM format.
* `--export-gmw <filename>`: Save the game in GM format, including weight data.

## Example

```bash
./nocq --rand 20 10 1 3 --weights -10 10 --noc --parity --mean-payoff --init 10
```

This execution generates a random game consisting of $n=20$ vertices with a maximum priority of $p=10$ and an out-degree for each vertex ranging between 1 and 3. It assigns random edge weights within the interval $[-10, 10]$ and utilizes the NOC (No-Opponent-Cycle) propagator to solve the game under both Parity and Mean-Payoff conditions simultaneously. The solver specifically computes the winning regions and strategies starting from the designated initial vertex 10.