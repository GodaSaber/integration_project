#include "integrator_cuda.h"
#include "rpn_parser.h"
#include <cuda_runtime.h>
#include <cmath>
#include <iostream>
#include <vector>
#include "rpn_eval.h"

// جهاز التقييم
__device__ double eval_rpn_device(const RPNToken* tokens, int n_tokens, double x) {
    double stack[64];
    int top = -1;
    for (int i = 0; i < n_tokens; ++i) {
        switch (tokens[i].type) {
            case TokenType::NUM: stack[++top] = tokens[i].value; break;
            case TokenType::VAR: stack[++top] = x; break;
            case TokenType::ADD: stack[top-1] += stack[top]; --top; break;
            case TokenType::SUB: stack[top-1] -= stack[top]; --top; break;
            case TokenType::MUL: stack[top-1] *= stack[top]; --top; break;
            case TokenType::DIV: stack[top-1] /= stack[top]; --top; break;
            case TokenType::POW: stack[top-1] = pow(stack[top-1], stack[top]); --top; break;
            case TokenType::SIN: stack[top] = sin(stack[top]); break;
            case TokenType::COS: stack[top] = cos(stack[top]); break;
            case TokenType::TAN: stack[top] = tan(stack[top]); break;
            case TokenType::EXP: stack[top] = exp(stack[top]); break;
            case TokenType::LOG: stack[top] = log(stack[top]); break;
        }
    }
    return stack[top];
}

// Kernel Riemann
__global__ void riemann_kernel(const RPNToken* tokens, int n_tokens,
                               double a, double dx, long long n,
                               double* partial_sums) {
    extern __shared__ double sdata[];
    long long tid = blockIdx.x * blockDim.x + threadIdx.x;
    long long stride = gridDim.x * blockDim.x;
    int lx = threadIdx.x;

    double sum = 0.0;
    for (long long i = tid; i < n; i += stride) {
        double x = a + (i + 0.5) * dx;   // midpoint rule
        sum += eval_rpn_device(tokens, n_tokens, x);
    }
    sdata[lx] = sum;
    __syncthreads();

    for (int s = blockDim.x/2; s > 0; s >>= 1) {
        if (lx < s) sdata[lx] += sdata[lx + s];
        __syncthreads();
    }
    if (lx == 0) partial_sums[blockIdx.x] = sdata[0];
}

// Kernel Simpson (يحسب المجموع الداخلي بأوزان)
__global__ void simpson_kernel(const RPNToken* tokens, int n_tokens,
                               double a, double dx, long long n,
                               double* partial_sums) {
    extern __shared__ double sdata[];
    long long tid = blockIdx.x * blockDim.x + threadIdx.x;
    long long stride = gridDim.x * blockDim.x;
    int lx = threadIdx.x;

    double sum = 0.0;
    for (long long i = tid + 1; i < n; i += stride) {
        double x = a + i * dx;
        double f = eval_rpn_device(tokens, n_tokens, x);
        double w = (i % 2 == 0) ? 2.0 : 4.0;
        sum += w * f;
    }
    sdata[lx] = sum;
    __syncthreads();

    for (int s = blockDim.x/2; s > 0; s >>= 1) {
        if (lx < s) sdata[lx] += sdata[lx + s];
        __syncthreads();
    }
    if (lx == 0) partial_sums[blockIdx.x] = sdata[0];
}

double integrate_cuda(double a, double b, long long n,
                      const std::string& function_str,
                      const std::string& method,
                      int block_size) {
    // تحويل التعبير إلى RPN (على الـ Host)
    CompiledRPN rpn = compile_expression(function_str);
    int n_tokens = rpn.tokens.size();

    // نسخ RPN إلى GPU
    RPNToken* d_tokens;
    cudaMalloc(&d_tokens, n_tokens * sizeof(RPNToken));
    cudaMemcpy(d_tokens, rpn.tokens.data(), n_tokens * sizeof(RPNToken),
               cudaMemcpyHostToDevice);

    // حساب dx
    double dx = (b - a) / n;

    // تحديد حجم الشبكة
    int threads = block_size;
    int blocks = (n + threads - 1) / threads;
    if (blocks > 65535) blocks = 65535;

    double* d_partial;
    cudaMalloc(&d_partial, blocks * sizeof(double));
    cudaMemset(d_partial, 0, blocks * sizeof(double));

    size_t shared = threads * sizeof(double);

    // اختيار kernel حسب method
    if (method == "riemann") {
        riemann_kernel<<<blocks, threads, shared>>>(d_tokens, n_tokens, a, dx, n, d_partial);
    } else if (method == "simpson") {
        // نضمن أن n زوجي
        if (n % 2 != 0) {
            n++;
            dx = (b - a) / n;
        }
        simpson_kernel<<<blocks, threads, shared>>>(d_tokens, n_tokens, a, dx, n, d_partial);
    } else {
        std::cerr << "Unknown method for CUDA: " << method << std::endl;
        cudaFree(d_tokens);
        cudaFree(d_partial);
        return 0.0;
    }

    cudaDeviceSynchronize();
    // نسخ النتائج الجزئية إلى الـ Host
    std::vector<double> h_partial(blocks);
    cudaMemcpy(h_partial.data(), d_partial, blocks * sizeof(double),
               cudaMemcpyDeviceToHost);

    double total = 0.0;
    for (double val : h_partial) total += val;

    if (method == "riemann") {
        total *= dx;
    } else { // simpson
        // endpoints
        double fa = eval_rpn(rpn, a);
        double fb = eval_rpn(rpn, b);
        total = (total + fa + fb) * dx / 3.0;
    }

    cudaFree(d_tokens);
    cudaFree(d_partial);

    return total;
}