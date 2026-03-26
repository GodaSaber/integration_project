#ifndef INTEGRATOR_OMP_H
#define INTEGRATOR_OMP_H

#include <string>

double integrate_omp(double a, double b, long long n,
                     const std::string& function_str,
                     const std::string& method,
                     int threads);

#endif