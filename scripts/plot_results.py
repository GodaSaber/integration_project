import matplotlib.pyplot as plt
import csv

data = {
    'n': [], 'impl': [], 'threads': [], 'block_size': [], 'avg_time': []
}
with open('results/results.csv', 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
        data['n'].append(int(row['n']))
        data['impl'].append(row['impl'])
        data['threads'].append(int(row['threads']))
        data['block_size'].append(int(row['block_size']))
        data['avg_time'].append(float(row['avg_time']))

times_ms = [t * 1000 for t in data['avg_time']]

# ----------------------------------------------------------------------
# Plot 1: Execution time vs n (Serial, OMP-4, CUDA-256)
# ----------------------------------------------------------------------
plt.figure(figsize=(8,5))

# Serial
ser_n = [data['n'][i] for i in range(len(data['n'])) if data['impl'][i] == 'serial']
ser_t = [times_ms[i] for i in range(len(data['n'])) if data['impl'][i] == 'serial']
if ser_n:
    plt.plot(ser_n, ser_t, 'o-', label='Serial')

# OpenMP with 4 threads
omp4_n = [data['n'][i] for i in range(len(data['n'])) if data['impl'][i] == 'omp' and data['threads'][i] == 4]
omp4_t = [times_ms[i] for i in range(len(data['n'])) if data['impl'][i] == 'omp' and data['threads'][i] == 4]
if omp4_n:
    plt.plot(omp4_n, omp4_t, 's-', label='OpenMP (4 threads)')

# CUDA with block size 256
cuda256_n = [data['n'][i] for i in range(len(data['n'])) if data['impl'][i] == 'cuda' and data['block_size'][i] == 256]
cuda256_t = [times_ms[i] for i in range(len(data['n'])) if data['impl'][i] == 'cuda' and data['block_size'][i] == 256]
if cuda256_n:
    plt.plot(cuda256_n, cuda256_t, '^-', label='CUDA (bs=256)')

plt.xlabel('Number of intervals (n)')
plt.ylabel('Time (ms)')
plt.title('Execution Time vs Problem Size')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('results/time_vs_n.png')
plt.show()

# ----------------------------------------------------------------------
# Plot 2: OpenMP Speedup vs Threads
# ----------------------------------------------------------------------
plt.figure(figsize=(8,5))
n_vals = sorted(set(data['n']))
for n_val in n_vals:
    # serial time
    ser_time = None
    for i in range(len(data['n'])):
        if data['impl'][i] == 'serial' and data['n'][i] == n_val:
            ser_time = data['avg_time'][i]
            break
    if ser_time is None:
        continue
    
    omp_times = {}
    for i in range(len(data['n'])):
        if data['impl'][i] == 'omp' and data['n'][i] == n_val:
            omp_times[data['threads'][i]] = data['avg_time'][i]
    if omp_times:
        threads = sorted(omp_times.keys())
        speedup = [ser_time / omp_times[t] for t in threads]
        plt.plot(threads, speedup, 'o-', label=f'n={n_val/1e6:.0f}M')

max_threads = max(data['threads']) if data['threads'] else 1
plt.plot([1, max_threads], [1, max_threads], 'k--', label='Ideal')
plt.xlabel('OpenMP Threads')
plt.ylabel('Speedup')
plt.title('OpenMP Speedup vs Threads')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('results/speedup.png')
plt.show()

# ----------------------------------------------------------------------
# Plot 3: CUDA Time vs Block Size
# ----------------------------------------------------------------------
plt.figure(figsize=(8,5))
cuda_data = {}
for i in range(len(data['n'])):
    if data['impl'][i] == 'cuda':
        n_val = data['n'][i]
        bs = data['block_size'][i]
        t = times_ms[i]
        if n_val not in cuda_data:
            cuda_data[n_val] = []
        cuda_data[n_val].append((bs, t))

for n_val in sorted(cuda_data.keys()):
    pts = sorted(cuda_data[n_val])
    bs_vals, t_vals = zip(*pts)
    plt.plot(bs_vals, t_vals, marker='s', label=f'n={n_val/1e6:.0f}M')

plt.xlabel('CUDA Block Size (threads/block)')
plt.ylabel('Time (ms)')
plt.title('CUDA Execution Time vs Block Size')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('results/cuda_block_size.png')
plt.show()

print("Plots saved to results/")