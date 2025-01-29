#include "vector.h"
#include "alloc.h"
#include <stdio.h>
#include <stdlib.h>

void vector_init(Vector *v, size_t capacity, VectorType type) {
  int size = 0;
  switch (type) {
  case CMDVECTOR:
    size = sizeof(Cmd);
    break;
  case TOKENVECTOR:
    size = sizeof(Token);
    break;
  case EXPRVECTOR:
    size = sizeof(Expr);
    break;
  case LVALUEVECTOR:
    size = sizeof(LValue);
    break;
  case TYPEVECTOR:
    size = sizeof(Type);
    break;
  case STMTVECTOR:
    size = sizeof(Stmt);
    break;
  case STRVECTOR:
    size = sizeof(char *);
    break;
  }
  v->type = type;
  v->data = alloc(capacity * size);
  v->size = 0;
  v->capacity = capacity;
}

void vector_append(Vector *v, void *item) {
  if (v->size == v->capacity) {
    int size = 0;
    switch (v->type) {
    case CMDVECTOR:
      size = sizeof(Cmd);
      break;
    case TOKENVECTOR:
      size = sizeof(Token);
      break;
    case EXPRVECTOR:
      size = sizeof(Expr);
      break;
    case LVALUEVECTOR:
      size = sizeof(LValue);
      break;
    case TYPEVECTOR:
      size = sizeof(Type);
      break;
    case STMTVECTOR:
      size = sizeof(Stmt);
      break;
    case STRVECTOR:
      size = sizeof(char *);
      break;
    }
    v->capacity *= 2;
    void **tmp = NULL;
    while (tmp == NULL)
      tmp = realloc(v->data, v->capacity * size);
    v->data = tmp;
  }

  switch (v->type) {
  case CMDVECTOR:
    ((Cmd **)v->data)[v->size++] = (Cmd *)item;
    break;
  case TOKENVECTOR:
    ((Token **)v->data)[v->size++] = (Token *)item;
    break;
  case EXPRVECTOR:
    ((Expr **)v->data)[v->size++] = (Expr *)item;
    break;
  case LVALUEVECTOR:
    ((LValue **)v->data)[v->size++] = (LValue *)item;
    break;
  case TYPEVECTOR:
    ((Type **)v->data)[v->size++] = (Type *)item;
    break;
  case STMTVECTOR:
    ((Stmt **)v->data)[v->size++] = (Stmt *)item;
    break;
  case STRVECTOR:
    ((char **)v->data)[v->size++] = (char *)item;
    break;
  }
}

Token *vector_get_token(Vector *v, int idx) {
  if (v->type != TOKENVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((Token **)v->data)[idx];
}

Cmd *vector_get_cmd(Vector *v, int idx) {
  if (v->type != CMDVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((Cmd **)v->data)[idx];
}

Expr *vector_get_expr(Vector *v, int idx) {
  if (v->type != EXPRVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((Expr **)v->data)[idx];
}

LValue *vector_get_lvalue(Vector *v, int idx) {
  if (v->type != LVALUEVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((LValue **)v->data)[idx];
}

Type *vector_get_type(Vector *v, int idx) {
  if (v->type != TYPEVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((Type **)v->data)[idx];
}

Stmt *vector_get_stmt(Vector *v, int idx) {
  if (v->type != STMTVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((Stmt **)v->data)[idx];
}

char *vector_get_str(Vector *v, int idx) {
  if (v->type != STMTVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((char **)v->data)[idx];
}
