#ifndef PARSE_EXPR_H
#define PARSE_EXPR_H
#include "ast.h"
#include "token.h"
#include "vector.h"

int parse_expr(Vector *tokens, int i, Expr *e);
int parse_expr_list(Vector *tokens, int i, ExprList *a);
int parse_expr_cont(Vector *tokens, int i, Expr *e);

#endif
