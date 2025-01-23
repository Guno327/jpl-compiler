#ifndef PARSER_H
#define PARSER_H
#include "ast.h"
#include "token.h"

Cmd** parse(Token** tokens);
Cmd* parse_cmd(Token** tokens, int start);
Expr* parse_expr(Token**, int start);
VarLValue parse_lvalue(Token** tokens, int start);

int peek_token(Token* t);
void expect_token(Token* t, int tok_type);

#endif
