#include "ast.h"
#include "token.h"
#include "vector.h"
#ifndef PARSE_EXPR_H
#define PARSE_EXPR_H

int parse_expr(vector *tokens, int i, expr *e);
int parse_expr_list(vector *tokens, int i, expr_list *a);
int parse_unary_level(vector *tokens, int i, expr *e);
int parse_index_level(vector *tokens, int i, expr *e);
int parse_mult_level(vector *tokens, int i, expr *e);
int parse_add_level(vector *tokens, int i, expr *e);
int parse_cmp_level(vector *tokens, int i, expr *e);
int parse_bool_level(vector *tokens, int i, expr *e);
int parse_base_level(vector *tokens, int i, expr *e);

#endif
