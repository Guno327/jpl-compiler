#ifndef PARSER_H
#define PARSER_H
#include "ast.h"
#include "token.h"
#include "vector.h"

Vector* parse(Vector* tokens);
int parse_cmd(Vector* tokens, int i, Cmd* c);
int parse_expr(Vector*, int i, Expr* e);
int parse_lvalue(Vector* tokens, int i, VarLValue* v);
int parse_array(Vector* tokens, int i, ArrayLiteralExpr* a);

int peek_token(Vector* tokens, int idx);
void expect_token(Vector* tokens, int idx, int tok_type);

#endif
