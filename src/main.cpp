#include <iostream>
#include <chrono>
#include <string>
#include <getopt.h>
#include <iomanip>
#include <fstream>
#include <vector>
#include <cmath>
#include <stdexcept>

#include "integrator_serial.h"
#include "integrator_omp.h"
#ifdef USE_CUDA
#include "integrator_cuda.h"
#endif

struct Options {
    std::string function = "x*x";
    double a = 0.0;
    double b = 1.0;
    long long n = 1000000;
    std::string method = "riemann";
    std::string impl = "serial";
    int threads = 1;
    int block_size = 256;
    int repeats = 5;
    std::string output = "";
    bool help = false;
    bool test = false;
};

void print_help() {
    std::cout << "Usage: ./integrate [options]\n"
              << "Options:\n"
              << "  --function EXPR   Mathematical expression (default: 'x*x')\n"
              << "  --a FLOAT         Lower limit (default: 0)\n"
              << "  --b FLOAT         Upper limit (default: 1)\n"
              << "  --n INT           Number of intervals (default: 1000000)\n"
              << "  --method METHOD   riemann, simpson (default: riemann)\n"
              << "  --impl TYPE       serial, omp, cuda (default: serial)\n"
              << "  --threads INT     OpenMP threads (default: 1)\n"
              << "  --block-size INT  CUDA block size (default: 256)\n"
              << "  --repeats INT     Repetitions for averaging (default: 5)\n"
              << "  --output FILE     Save CSV results (optional)\n"
              << "  --test            Run unit tests (integrity check)\n"
              << "  --help            Show this help\n";
}

double run_integration(const Options& opt) {
    if (opt.impl == "serial") {
        return integrate_serial(opt.a, opt.b, opt.n, opt.function, opt.method);
    } else if (opt.impl == "omp") {
        return integrate_omp(opt.a, opt.b, opt.n, opt.function, opt.method, opt.threads);
    } else if (opt.impl == "cuda") {
#ifdef USE_CUDA
        return integrate_cuda(opt.a, opt.b, opt.n, opt.function, opt.method, opt.block_size);
#else
        throw std::runtime_error("CUDA not available in this build (compile with -DUSE_CUDA)");
#endif
    }
    throw std::runtime_error("Unknown implementation: " + opt.impl);
}

// Unit tests using serial implementation
bool run_unit_tests() {
    const long long n = 10000000;  // large enough for good accuracy
    double tol = 1e-6;
    bool all_ok = true;

    // Test 1: ∫ x² dx from 0 to 1 = 1/3
    double res1 = integrate_serial(0, 1, n, "x^2", "riemann");
    double expected1 = 1.0/3.0;
    if (std::abs(res1 - expected1) > tol) {
        std::cerr << "Test 1 FAILED: ∫ x² [0,1] = " << res1 << " (expected " << expected1 << ")\n";
        all_ok = false;
    } else {
        std::cout << "Test 1 PASSED: ∫ x² [0,1] = " << res1 << "\n";
    }

    // Test 2: ∫ sin(x) dx from 0 to π = 2
    double pi = std::acos(-1.0);
    double res2 = integrate_serial(0, pi, n, "sin(x)", "riemann");
    double expected2 = 2.0;
    if (std::abs(res2 - expected2) > tol) {
        std::cerr << "Test 2 FAILED: ∫ sin(x) [0,π] = " << res2 << " (expected " << expected2 << ")\n";
        all_ok = false;
    } else {
        std::cout << "Test 2 PASSED: ∫ sin(x) [0,π] = " << res2 << "\n";
    }

    // Test 3: ∫ exp(x) dx from 0 to 1 = e - 1
    double res3 = integrate_serial(0, 1, n, "exp(x)", "riemann");
    double expected3 = std::exp(1.0) - 1.0;
    if (std::abs(res3 - expected3) > tol) {
        std::cerr << "Test 3 FAILED: ∫ exp(x) [0,1] = " << res3 << " (expected " << expected3 << ")\n";
        all_ok = false;
    } else {
        std::cout << "Test 3 PASSED: ∫ exp(x) [0,1] = " << res3 << "\n";
    }

    return all_ok;
}

int main(int argc, char** argv) {
    Options opt;

    static struct option long_options[] = {
        {"function", required_argument, 0, 'f'},
        {"a", required_argument, 0, 'a'},
        {"b", required_argument, 0, 'b'},
        {"n", required_argument, 0, 'n'},
        {"method", required_argument, 0, 'm'},
        {"impl", required_argument, 0, 'i'},
        {"threads", required_argument, 0, 't'},
        {"block-size", required_argument, 0, 'k'},
        {"repeats", required_argument, 0, 'r'},
        {"output", required_argument, 0, 'o'},
        {"test", no_argument, 0, 'T'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int c;
    while ((c = getopt_long(argc, argv, "f:a:b:n:m:i:t:k:r:o:Th", long_options, NULL)) != -1) {
        switch (c) {
            case 'f': opt.function = optarg; break;
            case 'a': opt.a = std::stod(optarg); break;
            case 'b': opt.b = std::stod(optarg); break;
            case 'n': opt.n = std::stoll(optarg); break;
            case 'm': opt.method = optarg; break;
            case 'i': opt.impl = optarg; break;
            case 't': opt.threads = std::stoi(optarg); break;
            case 'k': opt.block_size = std::stoi(optarg); break;
            case 'r': opt.repeats = std::stoi(optarg); break;
            case 'o': opt.output = optarg; break;
            case 'T': opt.test = true; break;
            case 'h': opt.help = true; break;
            default: print_help(); return 1;
        }
    }

    if (opt.help) { print_help(); return 0; }

    if (opt.test) {
        // Run unit tests and exit
        bool ok = run_unit_tests();
        return ok ? 0 : 1;
    }

    // Validate method and implementation
    if (opt.method != "riemann" && opt.method != "simpson") {
        std::cerr << "Error: method must be 'riemann' or 'simpson'\n";
        return 1;
    }
    if (opt.impl != "serial" && opt.impl != "omp" && opt.impl != "cuda") {
        std::cerr << "Error: impl must be 'serial', 'omp', or 'cuda'\n";
        return 1;
    }

    std::vector<double> times;
    double result = 0.0;
    bool error_occurred = false;

    for (int rep = 0; rep < opt.repeats; ++rep) {
        try {
            auto start = std::chrono::high_resolution_clock::now();
            result = run_integration(opt);
            auto end = std::chrono::high_resolution_clock::now();
            times.push_back(std::chrono::duration<double>(end - start).count());
        } catch (const std::exception& e) {
            std::cerr << "Error during integration (repeat " << rep+1 << "): " << e.what() << std::endl;
            error_occurred = true;
            break;
        }
    }

    if (error_occurred) {
        return 1;
    }

    // If all repeats succeeded, compute statistics
    double sum_t = 0.0;
    for (double t : times) sum_t += t;
    double mean_t = sum_t / times.size();
    double sq_diff = 0.0;
    for (double t : times) sq_diff += (t - mean_t) * (t - mean_t);
    double stddev_t = (times.size() > 1) ? std::sqrt(sq_diff / (times.size() - 1)) : 0.0;

    std::cout << std::fixed << std::setprecision(10);
    std::cout << "Result = " << result << std::endl;
    std::cout << "Average time = " << mean_t << " seconds\n";
    if (times.size() > 1) std::cout << "Std dev = " << stddev_t << " seconds\n";

    if (!opt.output.empty()) {
        std::ofstream ofs(opt.output, std::ios::app);
        if (!ofs) {
            std::cerr << "Cannot open output file: " << opt.output << std::endl;
            return 1;
        }
        ofs.seekp(0, std::ios::end);
        if (ofs.tellp() == 0) {
            ofs << "function,a,b,n,method,impl,threads,block_size,repeats,result,avg_time,stddev\n";
        }
        ofs << opt.function << "," << opt.a << "," << opt.b << "," << opt.n << ","
            << opt.method << "," << opt.impl << "," << opt.threads << ","
            << opt.block_size << "," << opt.repeats << "," << result << ","
            << mean_t << "," << stddev_t << "\n";
        ofs.close();
    }

    return 0;
}