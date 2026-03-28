# Numerical Integration with RPN – Serial, OpenMP & CUDA

This project computes the definite integral of a user‑supplied mathematical expression using Riemann (midpoint) sums. It provides three implementations:

- **Serial** – single‑threaded CPU baseline.
- **OpenMP** – shared‑memory parallel CPU version.
- **CUDA** – GPU accelerated version (designed and fully coded, though not executed in the current environment).

The expression is parsed into **Reverse Polish Notation (RPN)**, which is evaluated efficiently on both CPU and GPU.

---

## Features

- Expression parser supporting:  
  `+ - * / ^ sin cos tan exp log`, parentheses, variable `x`, and numeric constants.
- CLI with all required flags:  
  `--function`, `--a`, `--b`, `--n`, `--method {riemann,simpson}`,  
  `--impl {serial,omp,cuda}`, `--threads`, `--block-size`, `--repeats`, `--output`, `--help`.
- Experiment script (`run_experiments.sh`) runs all required conditions and produces a CSV file.
- Plotting script (`plot_results.py`) generates the required graphs (`time_vs_n.png`, `speedup.png`).
- Clean module structure: headers in `include/`, sources in `src/`, scripts in `scripts/`.

---

## Requirements

- **C++17** compiler (g++ recommended)
- **OpenMP** support (for the OMP version)
- **CUDA Toolkit** (optional, only if you want to compile the CUDA code)
- **Python 3** with `matplotlib` (for plotting)

### Command‑line Options

| Flag | Description |
|------|-------------|
| `--function` | Mathematical expression (default: `x*x`) |
| `--a`, `--b` | Integration limits |
| `--n` | Number of intervals |
| `--method` | `riemann` or `simpson` |
| `--impl` | `serial`, `omp`, or `cuda` |
| `--threads` | OpenMP thread count (for `--impl omp`) |
| `--block-size` | CUDA threads per block (for `--impl cuda`) |
| `--repeats` | Repetitions for averaging (default: 5) |
| `--output` | CSV file to append results |
| `--help` | Show help |

---
---

## 🔧 Full Build & Run Commands

### 🧠 Compile (Serial + OpenMP Only)

```bash
# Navigate to source directory
cd src

# Compile
g++ main.cpp integrator_serial.cpp integrator_omp.cpp rpn_parser.cpp rpn_eval.cpp -fopenmp -o integrator
```

▶️ Run Examples
```bash
# OpenMP (Riemann, x^3)
./integrator --impl omp --a 0 --b 1 --n 10000000 -f x^3 -m riemann -t 128

# Serial (Simpson, x^3)
./integrator --impl serial --a 0 --b 1 --n 10000 -f x^3 -m simpson
```

🚀 Compile with CUDA
```bash
# Go back to project root
cd ..

# Compile with CUDA support
nvcc -DUSE_CUDA -ccbin g++-11 -arch=sm_86 \
src/main.cpp \
src/integrator_serial.cpp \
src/integrator_omp.cpp \
src/integrator_cuda.cu \
src/rpn_parser.cpp \
src/rpn_eval.cpp \
-o integrate \
-Xcompiler -fopenmp \
-std=c++17
```

⚡ Run CUDA
```bash
./integrate --impl cuda --a 0 --b 1 --n 10000 -f x^3 -m simpson -k 128
```

🧪 Running Experiments

The experiment script runs all required configurations and saves results to results/results.csv.
```bash
chmod +x scripts/run_experiments.sh
./scripts/run_experiments.sh
```

After the experiments, generate the plots:

📊 Generate Plots
```bash
# Fix line endings (if needed)
sed -i 's/\r$//' scripts/plot_results.py

# Generate graphs
python3 scripts/plot_results.py
```

The graphs will be saved as `results/time_vs_n.png` and `results/speedup.png`.

> **Note:** The plot script requires `matplotlib`. Install it with `pip3 install matplotlib` or `sudo apt install python3-matplotlib`.

---

## Project Structure

```
.
Integration_project/
├── src/
│   ├── main.cpp
│   ├── integrator_serial.cpp / integrator_omp.cpp / integrator_serial.h 
│   ├── integrator_cuda.cu // integrator_omp.h / integrator_cuda.h
│   ├── rpn_parser.cpp / rpn_eval.cpp/ rpn_parser.h / rpn_eval.h
├── scripts/
│   ├── run_experiments.sh 
│   └── plot_results.py     
├── results/
    ├── results.csv/ time_vs_n.png/ speedup.png/ cuda_block_size.png
├── HPC_Integration_Report.docx
└── README.md

```

