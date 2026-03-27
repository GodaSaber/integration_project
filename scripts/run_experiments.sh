
#!/usr/bin/env bash
set -euo pipefail

# Configuration
BIN="./integrate"
OUT_DIR="results"
CSV="${OUT_DIR}/results.csv"
REPEATS=5
FUNC="x^2"
A=0
B=1
METHOD="riemann"

# Parameter ranges
THREADS_LIST=(1 2 4 8)
N_LIST=(1000000 5000000 10000000)
CUDA_BLOCK_SIZES=(128 256 512)

mkdir -p "${OUT_DIR}"
rm -f "${CSV}"

# CSV header
echo "function,a,b,n,method,impl,threads,block_size,repeats,result,avg_time,stddev" > "${CSV}"

log() { echo "[$(date +%H:%M:%S)] $*"; }

# Log system info (optional, for reproducibility)
log "=== System Info ==="
log "OS: $(uname -srm)"
log "CPU: $(grep 'model name' /proc/cpuinfo | head -1 | cut -d: -f2 | xargs)"
log "Cores: $(nproc)"
log "Compiler: $(g++ --version | head -1)"
if command -v nvidia-smi &>/dev/null; then
    log "GPU: $(nvidia-smi --query-gpu=name --format=csv,noheader)"
fi

log "=== Serial ==="
for N in "${N_LIST[@]}"; do
    log "  n=${N}"
    ${BIN} --function "${FUNC}" --a ${A} --b ${B} --n ${N} --method ${METHOD} --impl serial --repeats ${REPEATS} --output "${CSV}"
done

log "=== OpenMP ==="
for N in "${N_LIST[@]}"; do
    for T in "${THREADS_LIST[@]}"; do
        log "  n=${N} threads=${T}"
        ${BIN} --function "${FUNC}" --a ${A} --b ${B} --n ${N} --method ${METHOD} --impl omp --threads ${T} --repeats ${REPEATS} --output "${CSV}"
    done
done

log "=== CUDA ==="
for N in "${N_LIST[@]}"; do
    for BS in "${CUDA_BLOCK_SIZES[@]}"; do
        log "  n=${N} block_size=${BS}"
        ${BIN} --function "${FUNC}" --a ${A} --b ${B} --n ${N} --method ${METHOD} --impl cuda --block-size ${BS} --repeats ${REPEATS} --output "${CSV}"
    done
done

log "Done. Results saved to ${CSV}"