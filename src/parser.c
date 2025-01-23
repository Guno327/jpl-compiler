#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "error.h"

Vector* parse(Vector* tokens);
Cmd* parse_cmd(Token** tokens, int start);
Expr* parse_expr(Token**, int start);
VarLValue parse_lvalue(Token** tokens, int start);

int peek_token(Token* t){
  return t->type;
}

void expect_token(Token* t, int tok_type){
  if (t->type != tok_type){
    char* msg = malloc(BUFSIZ);
    sprintf(msg, "Unexpected token of type %d at %d", tok_type, t->start);
    parse_error(msg);
  }
}
