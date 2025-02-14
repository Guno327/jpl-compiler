#include "vector.h"
#include "alloc.h"
#include "vector_get.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  case STRUCTINFOVECTOR:
    size = sizeof(struct_info);
    break;
  case ARRAYINFOVECTOR:
    size = sizeof(array_info);
    break;
  case TVECTOR:
    size = sizeof(t);
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

bool vector_contains(vector *v, void *item) {
  for (int i = 0; i < v->size; i++) {
    switch (v->type) {
    case CMDVECTOR:;
      cmd *cur_cmd = vector_get_cmd(v, i);
      cmd *cmd_item = (cmd *)item;
      if (cur_cmd->node == cmd_item->node)
        return true;
      break;
    case TOKENVECTOR:;
      token *cur_token = vector_get_token(v, i);
      token *token_item = (token *)item;
      if (!strcmp(cur_token->text, token_item->text))
        return true;
      break;
    case EXPRVECTOR:;
      expr *cur_expr = vector_get_expr(v, i);
      expr *expr_item = (expr *)item;
      if (cur_expr->node == expr_item->node)
        return true;
      break;
    case LVALUEVECTOR:;
      lval *cur_lval = vector_get_lvalue(v, i);
      lval *lval_item = (lval *)item;
      if (cur_lval->node == lval_item->node)
        return true;
      break;
    case TYPEVECTOR:;
      type *cur_type = vector_get_type(v, i);
      type *type_item = (type *)item;
      if (cur_type->node == type_item->node)
        return true;
      break;
    case STMTVECTOR:;
      stmt *cur_stmt = vector_get_stmt(v, i);
      stmt *stmt_item = (stmt *)item;
      if (cur_stmt->node == stmt_item->node)
        return true;
      break;
    case BINDINGVECTOR:;
      binding *cur_binding = vector_get_binding(v, i);
      binding *binding_item = (binding *)item;
      if (cur_binding->lval == binding_item->lval &&
          cur_binding->type == binding_item->type)
        return true;
      break;
    case STRUCTINFOVECTOR:;
      struct_info *cur_struct_info = vector_get_struct_info(v, i);
      struct_info *struct_info_item = (struct_info *)item;
      if (cur_struct_info->name == struct_info_item->name)
        return true;
      break;
    case ARRAYINFOVECTOR:;
      array_info *cur_array_info = vector_get_array_info(v, i);
      array_info *array_info_item = (array_info *)item;
      if (cur_array_info->type == array_info_item->type)
        return true;
      break;
    case TVECTOR:;
      t *cur_t = vector_get_t(v, i);
      t *t_item = (t *)item;
      if (cur_t->type == t_item->type && cur_t->info == t_item->info)
        return true;
      break;
    case STRVECTOR:;
      char *cur_str = vector_get_str(v, i);
      char *str_item = (char *)item;
      if (!strcmp(cur_str, str_item))
        return true;
      break;
    }
  }
  return false;
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
    case STRUCTINFOVECTOR:
      size = sizeof(struct_info);
      break;
    case ARRAYINFOVECTOR:
      size = sizeof(array_info);
      break;
    case TVECTOR:
      size = sizeof(t);
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
  case STRUCTINFOVECTOR:
    ((struct_info **)v->data)[v->size++] = (struct_info *)item;
    break;
  case ARRAYINFOVECTOR:
    ((array_info **)v->data)[v->size++] = (array_info *)item;
    break;
  case TVECTOR:
    ((t **)v->data)[v->size++] = (t *)item;
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

t *vector_get_t(vector *v, int idx) {
  if (v->type != TVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((t **)v->data)[idx];
}

char *vector_get_str(vector *v, int idx) {
  if (v->type != STRVECTOR)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  return ((char **)v->data)[idx];
}
