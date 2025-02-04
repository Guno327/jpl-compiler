#ifndef PRECEDENCE_H
#define PRECEDENCE_H
#include "vector.h"
#include "ast.h"

int parse_top_level(Vector* tokens, int i, Expr *e);
int parse_index_level(Vector* tokens, int i, Expr *e);
int parse_unary_level(Vector* tokens, int i, Expr *e);
int parse_mult_level(Vector* tokens, int i, Expr *e);
int parse_add_level(Vector* tokens, int i, Expr *e);
int parse_cmp_level(Vector* tokens, int i, Expr *e);
int parse_bool_level(Vector* tokens, int i, Expr *e);

#endif
