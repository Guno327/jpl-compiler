#include "ast.h"
#include "token.h"
#include "vector.h"
#ifndef PARSE_EXPR_H
#define PARSE_EXPR_H

long parse_expr(vector *tokens, long i, expr *e);
long parse_expr_vec(vector *tokens, long i, vector *v);
long parse_unary_level(vector *tokens, long i, expr *e);
long parse_index_level(vector *tokens, long i, expr *e);
long parse_mult_level(vector *tokens, long i, expr *e);
long parse_add_level(vector *tokens, long i, expr *e);
long parse_cmp_level(vector *tokens, long i, expr *e);
long parse_bool_level(vector *tokens, long i, expr *e);
long parse_base_level(vector *tokens, long i, expr *e);

#endif
