#include "rpn_eval.h"
#include <cmath>
#include <stack>

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
                double b = st.top(); st.pop();
                double a = st.top(); st.pop();
                st.push(a + b);
                break;
            }
            case TokenType::SUB: {
                double b = st.top(); st.pop();
                double a = st.top(); st.pop();
                st.push(a - b);
                break;
            }
            case TokenType::MUL: {
                double b = st.top(); st.pop();
                double a = st.top(); st.pop();
                st.push(a * b);
                break;
            }
            case TokenType::DIV: {
                double b = st.top(); st.pop();
                double a = st.top(); st.pop();
                st.push(a / b);
                break;
            }
            case TokenType::POW: {
                double b = st.top(); st.pop();
                double a = st.top(); st.pop();
                st.push(std::pow(a, b));
                break;
            }
            case TokenType::SIN: {
                double a = st.top(); st.pop();
                st.push(std::sin(a));
                break;
            }
            case TokenType::COS: {
                double a = st.top(); st.pop();
                st.push(std::cos(a));
                break;
            }
            case TokenType::TAN: {
                double a = st.top(); st.pop();
                st.push(std::tan(a));
                break;
            }
            case TokenType::EXP: {
                double a = st.top(); st.pop();
                st.push(std::exp(a));
                break;
            }
            case TokenType::LOG: {
                double a = st.top(); st.pop();
                st.push(std::log(a));
                break;
            }
        }
    }
    return st.top();
}