#include "ast.h"
#include "token.h"
#include <stddef.h>
#ifndef VECTOR_H
#define VECTOR_H

typedef enum {
  TOKENVECTOR,
  CMDVECTOR,
  EXPRVECTOR,
  LVALUEVECTOR,
  TYPEVECTOR,
  STMTVECTOR,
  STRVECTOR,
  BINDINGVECTOR,
} vector_t;

typedef struct {
  vector_t type;
  void **data;
  size_t size;
  size_t capacity;
} vector;

void vector_init(vector *v, size_t capacity, vector_t type);
void vector_append(vector *v, void *item);

token *vector_get_token(vector *v, int idx);
cmd *vector_get_cmd(vector *v, int idx);
expr *vector_get_expr(vector *v, int idx);
lval *vector_get_lvalue(vector *v, int idx);
type *vector_get_type(vector *v, int idx);
stmt *vector_get_stmt(vector *v, int idx);
char *vector_get_str(vector *v, int idx);
binding *vector_get_binding(vector *v, int idx);

#endif
