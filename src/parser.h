#ifndef PARSER_H
#define PARSER_H
#include "ast.h"
#include "token.h"
#include "vector.h"

Vector* parse(Vector* tokens);
Cmd* parse_cmd(Vector* tokens, int* start);
Expr* parse_expr(Vector*, int* start);
VarLValue parse_lvalue(Vector* tokens, int* start);

int peek_token(Vector* tokens, int idx);
void expect_token(Vector* tokens, int idx, int tok_type);

#endif
