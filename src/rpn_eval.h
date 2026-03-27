#ifndef RPN_EVAL_H
#define RPN_EVAL_H

#include "rpn_parser.h"

#pragma once

double eval_rpn(const CompiledRPN& rpn, double x);
#endif