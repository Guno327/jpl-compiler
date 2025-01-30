#ifndef VECTOR_H
#define VECTOR_H
#include "ast.h"
#include "token.h"
#include <stddef.h>

typedef enum {
  TOKENVECTOR,
  CMDVECTOR,
  EXPRVECTOR,
  LVALUEVECTOR,
  TYPEVECTOR,
  STMTVECTOR,
  STRVECTOR,
  BINDINGVECTOR,
} VectorType;

typedef struct {
  VectorType type;
  void **data;
  size_t size;
  size_t capacity;
} Vector;

void vector_init(Vector *v, size_t capacity, VectorType type);
void vector_append(Vector *v, void *item);

Token *vector_get_token(Vector *v, int idx);
Cmd *vector_get_cmd(Vector *v, int idx);
Expr *vector_get_expr(Vector *v, int idx);
LValue *vector_get_lvalue(Vector *v, int idx);
Type *vector_get_type(Vector *v, int idx);
Stmt *vector_get_stmt(Vector *v, int idx);
char *vector_get_str(Vector *v, int idx);
Binding *vector_get_binding(Vector *v, int idx);

#endif
