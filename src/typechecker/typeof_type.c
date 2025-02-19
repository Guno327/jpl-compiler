#include "alloc.h"
#include "compiler_error.h"
#include "ctx.h"
#include "typecheck.h"
#include "vector.h"
#include "vector_get.h"
#include <stdio.h>
#include <string.h>

t *typeof_type(type *type, ctx *c) {
  t *result = alloc(sizeof(t));
  switch (type->type) {
  case INTTYPE:
    result->type = INT_T;
    result->info = NULL;
    break;
  case FLOATTYPE:
    result->type = FLOAT_T;
    result->info = NULL;
    break;
  case BOOLTYPE:
    result->type = BOOL_T;
    result->info = NULL;
    break;
  case VOIDTYPE:
    result->type = VOID_T;
    result->info = NULL;
    break;
  case ARRAYTYPE:;
    array_type *a_t = (array_type *)type->node;
    result->type = ARRAY_T;

    array_info *a_info = alloc(sizeof(array_info));
    a_info->type = typeof_type(a_t->type, c);
    a_info->rank = a_t->rank;
    result->info = a_info;
    break;
  case STRUCTTYPE:;
    struct_type *s_t = (struct_type *)type->node;
    struct_info *found = NULL;
    for (int i = 0; i < c->structs->size; i++) {
      struct_info *cur = vector_get_struct_info(c->structs, i);
      if (!strcmp(cur->name, s_t->var)) {
        found = cur;
        break;
      }
    }
    if (found == NULL) {
      char *msg = alloc(BUFSIZ);
      sprintf(msg,
              "struct of name '%s' is not "
              "declared at %d\n",
              s_t->var, s_t->start);
      typecheck_error(msg, s_t->start);
    }

    result->type = STRUCT_T;
    result->info = found;
    break;
  }
  return result;
}
