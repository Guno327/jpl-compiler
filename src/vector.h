#ifndef VECTOR_H
#define VECTOR_H

#include "token.h"
#include "ast.h"
#include <stddef.h>

typedef enum {TOKEN, CMD, EXPR} VectorType;
typedef struct {
  VectorType type;
  Token** data;
  size_t size;
  size_t capacity;
} Vector;

void vector_init(Vector* v, size_t capacity, VectorType type);
void vector_append(Vector* v, void* item);

void vector_free(Vector* v);

Token* vector_get_token(Vector* v, int idx);
Cmd* vector_get_cmd(Vector* v, int idx);
Expr* vector_get_expr(Vector* v, int idx);

#endif
