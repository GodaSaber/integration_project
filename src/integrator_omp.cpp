#include "integrator_omp.h"
#include "rpn_parser.h"
#include "rpn_eval.h"
#include <omp.h>
#include <iostream>

double integrate_omp(double a, double b, long long n,
                     const std::string& function_str,
                     const std::string& method,
                     int threads) {
    CompiledRPN rpn = compile_expression(function_str);
    double dx = (b - a) / n;
    double sum = 0.0;

    if (method == "riemann") {
        #pragma omp parallel num_threads(threads) reduction(+:sum)
        {
            #pragma omp for
            for (long long i = 0; i < n; ++i) {
                double x = a + (i + 0.5) * dx;
                sum += eval_rpn(rpn, x);
            }
        }
        return sum * dx;
    }
    else if (method == "simpson") {
        if (n % 2 != 0) {
            #pragma omp single
            std::cerr << "Warning: Simpson's rule requires even n. Increasing n by 1.\n";
            n++;
            dx = (b - a) / n;
        }
        double sum_odd = 0.0, sum_even = 0.0;
        #pragma omp parallel num_threads(threads) reduction(+:sum_odd,sum_even)
        {
            #pragma omp for
            for (long long i = 1; i < n; i += 2) {
                double x = a + i * dx;
                sum_odd += eval_rpn(rpn, x);
            }
            #pragma omp for
            for (long long i = 2; i < n; i += 2) {
                double x = a + i * dx;
                sum_even += eval_rpn(rpn, x);
            }
        }
        double fa = eval_rpn(rpn, a);
        double fb = eval_rpn(rpn, b);
        return (dx / 3.0) * (fa + fb + 4.0 * sum_odd + 2.0 * sum_even);
    }
    else {
        std::cerr << "Unknown method.\n";
        return 0.0;
    }
}