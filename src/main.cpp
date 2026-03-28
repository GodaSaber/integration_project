#include <iostream>
#include <chrono>
#include <string>
#include <getopt.h>
#include <iomanip>
#include <fstream>
#include <vector>
#include <cmath>

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
            std::cout << "CUDA not enabled in this build\n";
            return 0;
        #endif
    }
    return 0.0;
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
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int c;
    while ((c = getopt_long(argc, argv, "f:a:b:n:m:i:t:k:r:o:h", long_options, NULL)) != -1) {
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
            case 'h': opt.help = true; break;
            default: print_help(); return 1;
        }
    }

    if (opt.help) { print_help(); return 0; }

    if (opt.method != "riemann" && opt.method != "simpson") {
        std::cerr << "Error: method must be 'riemann' or 'simpson'\n"; return 1;
    }
    if (opt.impl != "serial" && opt.impl != "omp" && opt.impl != "cuda") {
        std::cerr << "Error: impl must be 'serial', 'omp', or 'cuda'\n"; return 1;
    }

    std::vector<double> times;
    double result = 0.0;

    for (int rep = 0; rep < opt.repeats; ++rep) {
        auto start = std::chrono::high_resolution_clock::now();
        result = run_integration(opt);
        auto end = std::chrono::high_resolution_clock::now();
        times.push_back(std::chrono::duration<double>(end - start).count());
    }

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
        if (!ofs) { std::cerr << "Cannot open output file: " << opt.output << std::endl; return 1; }
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


// #include <iostream>
// #include <chrono>
// #include <string>
// #include <cstring>
// #include <getopt.h>
// #include <cstdlib>
// #include <iomanip>
// #include <fstream>
// #include <vector>
// #include <cmath>
// #include "integrator_serial.h"
// #include "integrator_omp.h"

// struct Options {
//     std::string function = "x*x";
//     double a = 0.0;
//     double b = 1.0;
//     long long n = 1000000;
//     std::string method = "riemann";
//     std::string impl = "serial";
//     int threads = 1;
//     int repeats = 5;
//     std::string output = "";
//     bool help = false;
// };

// void print_help() {
//     std::cout << "Usage: ./integrate [options]\n"
//               << "Options:\n"
//               << "  --function EXPR   Mathematical expression (default: 'x*x')\n"
//               << "  --a FLOAT         Lower limit (default: 0)\n"
//               << "  --b FLOAT         Upper limit (default: 1)\n"
//               << "  --n INT           Number of intervals (default: 1000000)\n"
//               << "  --method METHOD   Integration method: riemann, simpson (default: riemann)\n"
//               << "  --impl TYPE        Implementation: serial, omp (default: serial)\n"
//               << "  --threads INT      Number of OpenMP threads (default: 1)\n"
//               << "  --repeats INT      Number of repetitions for averaging (default: 5)\n"
//               << "  --output FILE      Save results to CSV file (optional)\n"
//               << "  --help             Show this help\n";
// }

// double run_integration(const Options& opt) {
//     if (opt.impl == "serial") {
//         return integrate_serial(opt.a, opt.b, opt.n, opt.function, opt.method);
//     } else if (opt.impl == "omp") {
//         return integrate_omp(opt.a, opt.b, opt.n, opt.function, opt.method, opt.threads);
//     } else {
//         std::cerr << "Unsupported implementation: " << opt.impl << " (only serial/omp available)\n";
//         return 0.0;
//     }
// }

// int main(int argc, char** argv) {
//     Options opt;

//     static struct option long_options[] = {
//         {"function", required_argument, 0, 'f'},
//         {"a", required_argument, 0, 'a'},
//         {"b", required_argument, 0, 'b'},
//         {"n", required_argument, 0, 'n'},
//         {"method", required_argument, 0, 'm'},
//         {"impl", required_argument, 0, 'i'},
//         {"threads", required_argument, 0, 't'},
//         {"repeats", required_argument, 0, 'r'},
//         {"output", required_argument, 0, 'o'},
//         {"help", no_argument, 0, 'h'},
//         {0, 0, 0, 0}
//     };

//     int c;
//     while ((c = getopt_long(argc, argv, "f:a:b:n:m:i:t:r:o:h", long_options, NULL)) != -1) {
//         switch (c) {
//             case 'f': opt.function = optarg; break;
//             case 'a': opt.a = std::stod(optarg); break;
//             case 'b': opt.b = std::stod(optarg); break;
//             case 'n': opt.n = std::stoll(optarg); break;
//             case 'm': opt.method = optarg; break;
//             case 'i': opt.impl = optarg; break;
//             case 't': opt.threads = std::stoi(optarg); break;
//             case 'r': opt.repeats = std::stoi(optarg); break;
//             case 'o': opt.output = optarg; break;
//             case 'h': opt.help = true; break;
//             default: print_help(); return 1;
//         }
//     }

//     if (opt.help) { print_help(); return 0; }

//     if (opt.method != "riemann" && opt.method != "simpson") {
//         std::cerr << "Error: method must be 'riemann' or 'simpson'\n"; return 1;
//     }
//     if (opt.impl != "serial" && opt.impl != "omp") {
//         std::cerr << "Error: impl must be 'serial' or 'omp'\n"; return 1;
//     }

//     std::vector<double> times;
//     double result = 0.0;

//     for (int rep = 0; rep < opt.repeats; ++rep) {
//         auto start = std::chrono::high_resolution_clock::now();
//         result = run_integration(opt);
//         auto end = std::chrono::high_resolution_clock::now();
//         times.push_back(std::chrono::duration<double>(end - start).count());
//     }

//     double sum_t = 0.0;
//     for (double t : times) sum_t += t;
//     double mean_t = sum_t / times.size();
//     double sq_diff = 0.0;
//     for (double t : times) sq_diff += (t - mean_t) * (t - mean_t);
//     double stddev_t = (times.size() > 1) ? std::sqrt(sq_diff / (times.size() - 1)) : 0.0;

//     std::cout << std::fixed << std::setprecision(10);
//     std::cout << "Result = " << result << std::endl;
//     std::cout << "Average time = " << mean_t << " seconds\n";
//     if (times.size() > 1) std::cout << "Std dev = " << stddev_t << " seconds\n";

//     if (!opt.output.empty()) {
//         std::ofstream ofs(opt.output, std::ios::app);
//         if (!ofs) { std::cerr << "Cannot open output file: " << opt.output << std::endl; return 1; }
//         ofs.seekp(0, std::ios::end);
//         if (ofs.tellp() == 0) {
//             ofs << "function,a,b,n,method,impl,threads,repeats,result,avg_time,stddev\n";
//         }
//         ofs << opt.function << "," << opt.a << "," << opt.b << "," << opt.n << ","
//             << opt.method << "," << opt.impl << "," << opt.threads << ","
//             << opt.repeats << "," << result << "," << mean_t << "," << stddev_t << "\n";
//         ofs.close();
//     }

//     return 0;
// }
