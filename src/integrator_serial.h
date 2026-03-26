#ifndef INTEGRATOR_SERIAL_H
#define INTEGRATOR_SERIAL_H

#include <string>

double integrate_serial(double a, double b, long long n,
                        const std::string &function_str,
                        const std::string &method);

#endif