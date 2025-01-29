#ifndef PARSER_H
#define PARSER_H
#include "ast.h"
#include "token.h"
#include "vector.h"

CmdVector *parse(TokenVector *tokens);
int parse_cmd(TokenVector *tokens, int i, Cmd *c);
int parse_expr(TokenVector *, int i, Expr *e);
int parse_lvalue(TokenVector *tokens, int i, LValue *v);
int parse_array(TokenVector *tokens, int i, ArrayLiteralExpr *a);
int parse_type(TokenVector *tokens, int i, Type *t);
int parse_type_arr(TokenVector *tokens, int i, TypeArr *ta);
int parse_stmt(TokenVector *tokens, int i, Type *t);
int parse_binding(TokenVector *tokens, int i, Type *t);

int peek_token(TokenVector *tokens, int idx);
void expect_token(TokenVector *tokens, int idx, int tok_type);

#endif
