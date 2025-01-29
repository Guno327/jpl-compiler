#ifndef PARSE_LVALUE_H
#define PARSE_LVALUE_H
#include "ast.h"
#include "vector.h"

int parse_lvalue(Vector *tokens, int i, LValue *v);

#endif
