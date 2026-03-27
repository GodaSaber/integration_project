#ifndef INTEGRATOR_CUDA_H
#define INTEGRATOR_CUDA_H

#include <string>

double integrate_cuda(double a, double b, long long n,
                      const std::string& function_str,
                      const std::string& method,
                      int block_size);

#endif