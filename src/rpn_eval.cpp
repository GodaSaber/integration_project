#include "rpn_eval.h"
#include <cmath>
#include <stack>
#include <stdexcept>
#include <iostream>

double eval_rpn(const CompiledRPN& rpn, double x) {
    std::stack<double> st;
    for (const auto& tok : rpn.tokens) {
        switch (tok.type) {
            case TokenType::NUM:
                st.push(tok.value);
                break;
            case TokenType::VAR:
                st.push(x);
                break;
            case TokenType::ADD: {
                if (st.size() < 2) throw std::runtime_error("Not enough operands for ADD");
                double b = st.top(); st.pop();
                double a = st.top(); st.pop();
                st.push(a + b);
                break;
            }
            case TokenType::SUB: {
                if (st.size() < 2) throw std::runtime_error("Not enough operands for SUB");
                double b = st.top(); st.pop();
                double a = st.top(); st.pop();
                st.push(a - b);
                break;
            }
            case TokenType::MUL: {
                if (st.size() < 2) throw std::runtime_error("Not enough operands for MUL");
                double b = st.top(); st.pop();
                double a = st.top(); st.pop();
                st.push(a * b);
                break;
            }
            case TokenType::DIV: {
                if (st.size() < 2) throw std::runtime_error("Not enough operands for DIV");
                double b = st.top(); st.pop();
                double a = st.top(); st.pop();
                if (b == 0.0) throw std::runtime_error("Division by zero");
                st.push(a / b);
                break;
            }
            case TokenType::POW: {
                if (st.size() < 2) throw std::runtime_error("Not enough operands for POW");
                double b = st.top(); st.pop();
                double a = st.top(); st.pop();
                st.push(std::pow(a, b));
                break;
            }
            case TokenType::SIN: {
                if (st.empty()) throw std::runtime_error("Not enough operands for SIN");
                double a = st.top(); st.pop();
                st.push(std::sin(a));
                break;
            }
            case TokenType::COS: {
                if (st.empty()) throw std::runtime_error("Not enough operands for COS");
                double a = st.top(); st.pop();
                st.push(std::cos(a));
                break;
            }
            case TokenType::TAN: {
                if (st.empty()) throw std::runtime_error("Not enough operands for TAN");
                double a = st.top(); st.pop();
                st.push(std::tan(a));
                break;
            }
            case TokenType::EXP: {
                if (st.empty()) throw std::runtime_error("Not enough operands for EXP");
                double a = st.top(); st.pop();
                st.push(std::exp(a));
                break;
            }
            case TokenType::LOG: {
                if (st.empty()) throw std::runtime_error("Not enough operands for LOG");
                double a = st.top(); st.pop();
                if (a <= 0.0) throw std::runtime_error("log of non-positive number");
                st.push(std::log(a));
                break;
            }
        }
    }
    if (st.size() != 1) throw std::runtime_error("Invalid expression: stack size not 1");
    return st.top();
}