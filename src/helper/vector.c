#include "vector.h"
#include "safe.h"
#include "vector_get.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void vector_init(vector *v, long capacity, vector_t v_type) {
  int size = 0;
  switch (v_type) {
  case CMDVECTOR:
    size = sizeof(cmd *);
    break;
  case TOKENVECTOR:
    size = sizeof(token *);
    break;
  case EXPRVECTOR:
    size = sizeof(expr *);
    break;
  case LVALUEVECTOR:
    size = sizeof(lval *);
    break;
  case TYPEVECTOR:
    size = sizeof(type *);
    break;
  case STMTVECTOR:
    size = sizeof(stmt *);
    break;
  case BINDINGVECTOR:
    size = sizeof(binding *);
    break;
  case STRUCTINFOVECTOR:
    size = sizeof(struct_info *);
    break;
  case ARRAYINFOVECTOR:
    size = sizeof(array_info *);
    break;
  case FNINFOVECTOR:
    size = sizeof(fn_info *);
    break;
  case VARINFOVECTOR:
    size = sizeof(var_info *);
    break;
  case TVECTOR:
    size = sizeof(t *);
    break;
  case CFNVECTOR:
    size = sizeof(c_fn *);
    break;
  case CSTRUCTVECTOR:
    size = sizeof(c_struct *);
    break;
  case ASMFNVECTOR:
    size = sizeof(asm_fn *);
    break;
  case NUMVECTOR:
    size = sizeof(long);
    break;
  case STRVECTOR:
    size = sizeof(char *);
    break;
  }
  v->type = v_type;
  v->data = safe_alloc(capacity * size);
  v->size = 0;
  v->capacity = capacity;
}

void vector_append(vector *v, void *item) {
  if (v->size == v->capacity) {
    int size = 0;
    switch (v->type) {
    case CMDVECTOR:
      size = sizeof(cmd *);
      break;
    case TOKENVECTOR:
      size = sizeof(token *);
      break;
    case EXPRVECTOR:
      size = sizeof(expr *);
      break;
    case LVALUEVECTOR:
      size = sizeof(lval *);
      break;
    case TYPEVECTOR:
      size = sizeof(type *);
      break;
    case STMTVECTOR:
      size = sizeof(stmt *);
      break;
    case BINDINGVECTOR:
      size = sizeof(binding *);
      break;
    case STRUCTINFOVECTOR:
      size = sizeof(struct_info *);
      break;
    case ARRAYINFOVECTOR:
      size = sizeof(array_info *);
      break;
    case FNINFOVECTOR:
      size = sizeof(fn_info *);
      break;
    case VARINFOVECTOR:
      size = sizeof(var_info *);
      break;
    case TVECTOR:
      size = sizeof(t *);
      break;
    case CFNVECTOR:
      size = sizeof(c_fn *);
      break;
    case CSTRUCTVECTOR:
      size = sizeof(c_struct *);
      break;
    case ASMFNVECTOR:
      size = sizeof(asm_fn *);
      break;
    case NUMVECTOR:
      size = sizeof(long *);
      break;
    case STRVECTOR:
      size = sizeof(char **);
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
  case STRUCTINFOVECTOR:
    ((struct_info **)v->data)[v->size++] = (struct_info *)item;
    break;
  case ARRAYINFOVECTOR:
    ((array_info **)v->data)[v->size++] = (array_info *)item;
    break;
  case FNINFOVECTOR:
    ((fn_info **)v->data)[v->size++] = (fn_info *)item;
    break;
  case VARINFOVECTOR:
    ((var_info **)v->data)[v->size++] = (var_info *)item;
    break;
  case TVECTOR:
    ((t **)v->data)[v->size++] = (t *)item;
    break;
  case CFNVECTOR:
    ((c_fn **)v->data)[v->size++] = (c_fn *)item;
    break;
  case CSTRUCTVECTOR:
    ((c_struct **)v->data)[v->size++] = (c_struct *)item;
    break;
  case ASMFNVECTOR:
    ((asm_fn **)v->data)[v->size++] = (asm_fn *)item;
    break;
  case NUMVECTOR:
    ((long *)v->data)[v->size++] = (long)item;
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

struct_info *vector_get_struct_info(vector *v, int idx) {
  if (v->type != STRUCTINFOVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((struct_info **)v->data)[idx];
}

array_info *vector_get_array_info(vector *v, int idx) {
  if (v->type != ARRAYINFOVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((array_info **)v->data)[idx];
}

fn_info *vector_get_fn_info(vector *v, int idx) {
  if (v->type != FNINFOVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((fn_info **)v->data)[idx];
}

var_info *vector_get_var_info(vector *v, int idx) {
  if (v->type != VARINFOVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((var_info **)v->data)[idx];
}

t *vector_get_t(vector *v, int idx) {
  if (v->type != TVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((t **)v->data)[idx];
}

c_fn *vector_get_c_fn(vector *v, int idx) {
  if (v->type != CFNVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((c_fn **)v->data)[idx];
}

c_struct *vector_get_c_struct(vector *v, int idx) {
  if (v->type != CSTRUCTVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((c_struct **)v->data)[idx];
}

char *vector_get_str(vector *v, int idx) {
  if (v->type != STRVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((char **)v->data)[idx];
}

asm_fn *vector_get_asm_fn(vector *v, int idx) {
  if (v->type != ASMFNVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((asm_fn **)v->data)[idx];
}

long vector_get_num(vector *v, int idx) {
  if (v->type != NUMVECTOR)
    return 0;
  if (idx < 0 || idx > v->size - 1)
    return 0;
  return ((long *)v->data)[idx];
}
