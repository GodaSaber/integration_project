import matplotlib.pyplot as plt
import csv

data = {'n': [], 'impl': [], 'threads': [], 'avg_time': []}
with open('results/results.csv', 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
        data['n'].append(int(row['n']))
        data['impl'].append(row['impl'])
        data['threads'].append(int(row['threads']))
        data['avg_time'].append(float(row['avg_time']))

times_ms = [t * 1000 for t in data['avg_time']]

# 1. time vs n (serial, omp-4, cuda-256)
plt.figure(figsize=(8,5))
serial = [(data['n'][i], times_ms[i]) for i in range(len(data['n'])) if data['impl'][i] == 'serial']
if serial:
    n_vals, t_vals = zip(*serial)
    plt.plot(n_vals, t_vals, 'o-', label='Serial')

omp4 = [(data['n'][i], times_ms[i]) for i in range(len(data['n'])) if data['impl'][i] == 'omp' and data['threads'][i] == 4]
if omp4:
    n_vals, t_vals = zip(*omp4)
    plt.plot(n_vals, t_vals, 's-', label='OpenMP (4 threads)')

cuda256 = [(data['n'][i], times_ms[i]) for i in range(len(data['n'])) if data['impl'][i] == 'cuda' and data['threads'][i] == 256]  # block_size
if cuda256:
    n_vals, t_vals = zip(*cuda256)
    plt.plot(n_vals, t_vals, '^-', label='CUDA (bs=256)')

plt.xlabel('Number of intervals (n)')
plt.ylabel('Time (ms)')
plt.title('Execution Time vs Problem Size')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('results/time_vs_n.png')
plt.show()

# 2. speedup vs threads for OpenMP
plt.figure(figsize=(8,5))
n_vals = sorted(set(data['n']))
for n_val in n_vals:
    ser_time = next((data['avg_time'][i] for i in range(len(data['n'])) if data['impl'][i] == 'serial' and data['n'][i] == n_val), None)
    if ser_time is None: continue
    omp_data = [(data['threads'][i], data['avg_time'][i]) for i in range(len(data['n'])) if data['impl'][i] == 'omp' and data['n'][i] == n_val]
    if omp_data:
        omp_data.sort()
        threads = [t for t, _ in omp_data]
        times = [t for _, t in omp_data]
        speedup = [ser_time / t for t in times]
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

# 3. CUDA time vs block size
plt.figure(figsize=(8,5))
cuda_df = [(data['n'][i], data['threads'][i], times_ms[i]) for i in range(len(data['n'])) if data['impl'][i] == 'cuda']
if cuda_df:
    by_n = {}
    for n_val, bs, t in cuda_df:
        by_n.setdefault(n_val, []).append((bs, t))
    for n_val, pts in by_n.items():
        pts.sort()
        bs_vals, t_vals = zip(*pts)
        plt.plot(bs_vals, t_vals, marker='s', label=f'n={n_val/1e6:.0f}M')
    plt.xlabel('CUDA block size')
    plt.ylabel('Time (ms)')
    plt.title('CUDA Time vs Block Size')
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig('results/cuda_block_size.png')
    plt.show()

print("Plots saved to results/")