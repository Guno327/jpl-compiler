#ifndef PARSE_EXPR_H
#define PARSE_EXPR_H
#include "token.h"
#include "vector.h"

int parse_expr(TokenVector *tokens, int i, Expr *e);
int parse_array(TokenVector *tokens, int i, ArrayLiteralExpr *a);

#endif
