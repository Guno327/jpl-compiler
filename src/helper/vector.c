#include "vector.h"
#include "alloc.h"
#include <stdio.h>
#include <stdlib.h>

void vector_init(vector *v, size_t capacity, vector_t type) {
  int size = 0;
  switch (type) {
  case CMDVECTOR:
    size = sizeof(cmd);
    break;
  case TOKENVECTOR:
    size = sizeof(token);
    break;
  case EXPRVECTOR:
    size = sizeof(expr);
    break;
  case LVALUEVECTOR:
    size = sizeof(lval);
    break;
  case TYPEVECTOR:
    size = sizeof(type);
    break;
  case STMTVECTOR:
    size = sizeof(stmt);
    break;
  case BINDINGVECTOR:
    size = sizeof(binding);
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

void vector_append(vector *v, void *item) {
  if (v->size == v->capacity) {
    int size = 0;
    switch (v->type) {
    case CMDVECTOR:
      size = sizeof(cmd);
      break;
    case TOKENVECTOR:
      size = sizeof(token);
      break;
    case EXPRVECTOR:
      size = sizeof(expr);
      break;
    case LVALUEVECTOR:
      size = sizeof(lval);
      break;
    case TYPEVECTOR:
      size = sizeof(type);
      break;
    case STMTVECTOR:
      size = sizeof(stmt);
      break;
    case BINDINGVECTOR:
      size = sizeof(binding);
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
    ((cmd **)v->data)[v->size++] = (cmd *)item;
    break;
  case TOKENVECTOR:
    ((token **)v->data)[v->size++] = (token *)item;
    break;
  case EXPRVECTOR:
    ((expr **)v->data)[v->size++] = (expr *)item;
    break;
  case LVALUEVECTOR:
    ((lval **)v->data)[v->size++] = (lval *)item;
    break;
  case TYPEVECTOR:
    ((type **)v->data)[v->size++] = (type *)item;
    break;
  case STMTVECTOR:
    ((stmt **)v->data)[v->size++] = (stmt *)item;
    break;
  case BINDINGVECTOR:
    ((binding **)v->data)[v->size++] = (binding *)item;
    break;
  case STRVECTOR:
    ((char **)v->data)[v->size++] = (char *)item;
    break;
  }
}

token *vector_get_token(vector *v, int idx) {
  if (v->type != TOKENVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((token **)v->data)[idx];
}

cmd *vector_get_cmd(vector *v, int idx) {
  if (v->type != CMDVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((cmd **)v->data)[idx];
}

expr *vector_get_expr(vector *v, int idx) {
  if (v->type != EXPRVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((expr **)v->data)[idx];
}

lval *vector_get_lvalue(vector *v, int idx) {
  if (v->type != LVALUEVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((lval **)v->data)[idx];
}

type *vector_get_type(vector *v, int idx) {
  if (v->type != TYPEVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((type **)v->data)[idx];
}

stmt *vector_get_stmt(vector *v, int idx) {
  if (v->type != STMTVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((stmt **)v->data)[idx];
}

binding *vector_get_binding(vector *v, int idx) {
  if (v->type != BINDINGVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((binding **)v->data)[idx];
}

char *vector_get_str(vector *v, int idx) {
  if (v->type != STMTVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((char **)v->data)[idx];
}
