#include "parser.h"

Cmd** parse(Token** tokens);
Cmd* parse_cmd(Token** tokens, int start);
Expr* parse_expr(Token**, int start);
VarLValue parse_lvalue(Token** tokens, int start);

int peek_token(Token* t){
  return t->type;
}

void expect_token(Token* t, int tok_type){
  if (t->type != tok_type){

  }
}
