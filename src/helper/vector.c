#include "vector.h"
#include "alloc.h"
#include <stdio.h>
#include <stdlib.h>

void vector_init_token(TokenVector *v, size_t capacity) {
  v->data = alloc(capacity * sizeof(Token));
  v->size = 0;
  v->capacity = capacity;
}

void vector_init_cmd(CmdVector *v, size_t capacity) {
  v->data = malloc(capacity * sizeof(Cmd));
  v->size = 0;
  v->capacity = capacity;
}

void vector_init_expr(ExprVector *v, size_t capacity) {
  v->data = malloc(capacity * sizeof(Expr));
  v->size = 0;
  v->capacity = capacity;
}

void vector_append_token(TokenVector *v, Token *item) {
  if (v->size == v->capacity) {
    v->capacity *= 2;
    Token **tmp = NULL;
    while (tmp == NULL)
      tmp = realloc(v->data, v->capacity * sizeof(Token));
    v->data = tmp;
  }
  v->data[v->size++] = item;
}

void vector_append_cmd(CmdVector *v, Cmd *item) {
  if (v->size == v->capacity) {
    v->capacity *= 2;
    Cmd **tmp = NULL;
    while (tmp == NULL)
      tmp = realloc(v->data, v->capacity * sizeof(Cmd));
    v->data = tmp;
  }
  v->data[v->size++] = item;
}

void vector_append_expr(ExprVector *v, Expr *item) {
  if (v->size == v->capacity) {
    v->capacity *= 2;
    Expr **tmp = NULL;
    while (tmp == NULL)
      tmp = realloc(v->data, v->capacity * sizeof(Expr));
    v->data = tmp;
  }
  v->data[v->size++] = item;
}

Token *vector_get_token(TokenVector *v, int idx) {
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  Token *t = (Token *)v->data[idx];
  return t;
}

Cmd *vector_get_cmd(CmdVector *v, int idx) {
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  Cmd *c = (Cmd *)v->data[idx];
  return c;
}

Expr *vector_get_expr(ExprVector *v, int idx) {
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  Expr *c = (Expr *)v->data[idx];
  return c;
}
