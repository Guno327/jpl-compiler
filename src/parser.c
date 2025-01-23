#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "error.h"

Vector* parse(Vector* tokens){
  Vector* program = malloc(sizeof(Vector));
  vector_init(program, 128, sizeof(Cmd));

  int* i = malloc(sizeof(int));
  for(*i = 0; *i < tokens->size;){
    Cmd* c = parse_cmd(tokens, i);
    vector_append(program, c);
    expect_token(tokens, *i, NEWLINE);
    *i += 1;
  }
  return program;
}

Cmd* parse_cmd(Vector* tokens, int* start){
  Cmd* c = malloc(sizeof(Cmd));
  c->start = *start;
  switch (peek_token(tokens, *start)){
    case READ:
      expect_token(tokens, *start + 1, IMAGE);
      expect_token(tokens, *start + 2, STRING);
      expect_token(tokens, *start + 3, TO);
      expect_token(tokens, *start + 4, VARIABLE);
      ReadCmd* rc = malloc(sizeof(ReadCmd));
      rc->start = *start;
      rc->str = ((Token*)(tokens->data[*start+2]))->text;
    case WRITE:
    case LET:
    case ASSERT:
    case PRINT:
    case SHOW:
    case TIME:
    default:
  }  
  return c;
}

Expr* parse_expr(Vector*, int* start);
VarLValue parse_lvalue(Vector* tokens, int* start);

int peek_token(Vector* tokens, int idx){
  Token* t = (Token*)tokens->data[idx];
  return t->type;
}

void expect_token(Vector* tokens, int idx, int tok_type){
  Token* t = (Token*)tokens->data[idx];
  if (t->type != tok_type){
    char* msg = malloc(BUFSIZ);
    sprintf(msg, "Unexpected token of type %d at %d", tok_type, t->start);
    parse_error(msg);
  }
}
