#ifndef VECTOR_H
#define VECTOR_H

#include "token.h"
#include "ast.h"
#include <stddef.h>

typedef struct {
  Token** data;
  size_t size;
  size_t capacity;
} TokenVector;

typedef struct {
  Cmd** data;
  size_t size;
  size_t capacity;
} CmdVector;

typedef struct {
  Expr** data;
  size_t size;
  size_t capacity;
} ExprVector;

void vector_init_token(TokenVector* v, size_t capacity);
void vector_init_cmd(CmdVector* v, size_t capacity);
void vector_init_expr(ExprVector* v, size_t capacity);

void vector_append_token(TokenVector* v, Token* item);
void vector_append_cmd(CmdVector* v, Cmd* item);
void vector_append_expr(ExprVector* v, Expr* item);

Token* vector_get_token(TokenVector* v, int idx);
Cmd* vector_get_cmd(CmdVector* v, int idx);
Expr* vector_get_expr(ExprVector* v, int idx);

#endif
