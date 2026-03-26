#include "rpn_parser.h"
#include <stack>
#include <map>
#include <cctype>
#include <cmath>

static int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    if (op == '^') return 3;
    return 0;
}

static bool right_associative(char op) {
    return op == '^';
}

static void skip_spaces(const std::string& s, size_t& i) {
    while (i < s.size() && std::isspace(s[i])) ++i;
}

static double parse_number(const std::string& s, size_t& i) {
    size_t start = i;
    while (i < s.size() && (std::isdigit(s[i]) || s[i] == '.')) ++i;
    return std::stod(s.substr(start, i - start));
}

static std::string parse_identifier(const std::string& s, size_t& i) {
    size_t start = i;
    while (i < s.size() && (std::isalpha(s[i]) || std::isdigit(s[i]))) ++i;
    return s.substr(start, i - start);
}

CompiledRPN compile_expression(const std::string& expr) {
    std::vector<RPNToken> output;
    std::stack<char> ops;

    size_t i = 0;
    while (i < expr.size()) {
        skip_spaces(expr, i);
        if (i >= expr.size()) break;

        char c = expr[i];
        if (std::isdigit(c) || c == '.') {
            // رقم
            double num = parse_number(expr, i);
            output.push_back({TokenType::NUM, num});
            continue;
        }
        else if (c == 'x' || c == 'X') {
            // متغير x
            output.push_back({TokenType::VAR, 0.0});
            ++i;
            continue;
        }
        else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^') {
            // عامل ثنائي
            while (!ops.empty() && ops.top() != '(' &&
                   ((precedence(ops.top()) > precedence(c)) ||
                    (precedence(ops.top()) == precedence(c) && !right_associative(c)))) {
                output.push_back({static_cast<TokenType>(ops.top()), 0.0});
                ops.pop();
            }
            ops.push(c);
            ++i;
        }
        else if (c == '(') {
            ops.push('(');
            ++i;
        }
        else if (c == ')') {
            while (!ops.empty() && ops.top() != '(') {
                output.push_back({static_cast<TokenType>(ops.top()), 0.0});
                ops.pop();
            }
            if (!ops.empty() && ops.top() == '(') ops.pop();
            ++i;
        }
        else if (std::isalpha(c)) {
            std::string name = parse_identifier(expr, i);
            TokenType tt;
            if (name == "sin") tt = TokenType::SIN;
            else if (name == "cos") tt = TokenType::COS;
            else if (name == "tan") tt = TokenType::TAN;
            else if (name == "exp") tt = TokenType::EXP;
            else if (name == "log") tt = TokenType::LOG;
            else {
                continue;
            }
            ops.push(static_cast<char>(tt));  
        }
        else {
            ++i;
        }
    }

    while (!ops.empty()) {
        output.push_back({static_cast<TokenType>(ops.top()), 0.0});
        ops.pop();
    }

    CompiledRPN result;
    result.tokens = std::move(output);
    return result;
}