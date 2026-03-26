#ifndef RPN_PARSER_H
#define RPN_PARSER_H

#include <vector>
#include <string>
#include <cstdint>

enum class TokenType : uint8_t {
    NUM,       
    VAR,        
    ADD, SUB, MUL, DIV, POW,  
    SIN, COS, TAN, EXP, LOG    
};

struct RPNToken {
    TokenType type;
    double    value;    
};

struct CompiledRPN {
    std::vector<RPNToken> tokens;
};

CompiledRPN compile_expression(const std::string& expr);

#endif