#ifndef PARSE_EXPR_H
#define PARSE_EXPR_H
#include "ast.h"
#include "token.h"
#include "vector.h"

int parse_expr(Vector *tokens, int i, Expr *e);
int parse_expr_list(Vector *tokens, int i, ExprList *a);
int parse_index_level(Vector *tokens, int i, Expr *e);
int parse_unary_level(Vector *tokens, int i, Expr *e);
int parse_mult_level(Vector *tokens, int i, Expr *e);
int parse_add_level(Vector *tokens, int i, Expr *e);
int parse_cmp_level(Vector *tokens, int i, Expr *e);
int parse_bool_level(Vector *tokens, int i, Expr *e);
int parse_base_level(Vector *tokens, int i, Expr *e);

#endif
