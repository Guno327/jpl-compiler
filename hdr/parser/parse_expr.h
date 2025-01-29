#ifndef PARSE_EXPR_H
#define PARSE_EXPR_H
#include "ast.h"
#include "token.h"
#include "vector.h"

int parse_expr(TokenVector *tokens, int i, Expr *e);
int parse_expr_list(TokenVector *tokens, int i, ExprList *a);
int parse_expr_cont(TokenVector *tokens, int i, Expr *e);

#endif
