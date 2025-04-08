#include "ctx.h"
#include "safe.h"
#include "vector_get.h"
#include <stdio.h>
#include <string.h>

ctx *setup_ctx() {
  ctx *c = safe_alloc(sizeof(ctx));
  c->structs = safe_alloc(sizeof(vector));
  c->arrays = safe_alloc(sizeof(vector));
  c->fns = safe_alloc(sizeof(vector));
  c->vars = safe_alloc(sizeof(vector));

  vector_init(c->structs, 8, STRUCTINFOVECTOR);
  vector_init(c->arrays, 8, ARRAYINFOVECTOR);
  vector_init(c->fns, 8, FNINFOVECTOR);
  vector_init(c->vars, 8, VARINFOVECTOR);

  return c;
}

info *check_ctx(ctx *c, char *name) {
  long i;
  info *result = safe_alloc(sizeof(info));

  // check structs
  struct_info *s_info = NULL;
  for (i = 0; i < c->structs->size; i++) {
    struct_info *info = vector_get_struct_info(c->structs, i);
    if (!strcmp(info->name, name)) {
      s_info = info;
      break;
    }
  }
  if (s_info != NULL) {
    result->type = STRUCTINFO;
    result->node = s_info;
    return result;
  }

  // check arrays
  array_info *a_info = NULL;
  for (i = 0; i < c->arrays->size; i++) {
    array_info *info = vector_get_array_info(c->arrays, i);
    if (!strcmp(info->name, name)) {
      a_info = info;
      break;
    }
  }
  if (a_info != NULL) {
    result->type = ARRAYINFO;
    result->node = a_info;
    return result;
  }

  // check vars
  var_info *v_info = NULL;
  for (i = 0; i < c->vars->size; i++) {
    var_info *info = vector_get_var_info(c->vars, i);
    if (!strcmp(info->name, name)) {
      v_info = info;
      break;
    }
  }
  if (v_info != NULL) {
    result->type = VARINFO;
    result->node = v_info;
    return result;
  }

  // check functions
  fn_info *f_info = NULL;
  for (i = 0; i < c->fns->size; i++) {
    fn_info *info = vector_get_fn_info(c->fns, i);
    if (!strcmp(info->name, name)) {
      f_info = info;
      break;
    }
  }
  if (f_info != NULL) {
    result->type = FNINFO;
    result->node = f_info;
    return result;
  }

  // Not found at this level, check parent
  free(result);
  result = NULL;
  if (c->parent != NULL)
    result = check_ctx(c->parent, name);
  return result;
}

char *ctx_to_str(ctx *c) {
  char *result = safe_alloc(BUFSIZ);
  long i = 0;
  if (c->parent != NULL) {
    result = safe_realloc_str(result, BUFSIZ * 2);
    char *parent = ctx_to_str(c->parent);
    sprintf(result, "PARENT\n%s\n", parent);
    free(parent);
  }

  strcat(result, "FNS\n");
  for (i = 0; i < c->fns->size; i++) {
    fn_info *cur = vector_get_fn_info(c->fns, i);
    strcat(result, cur->name);
    strcat(result, "\n");
  }

  strcat(result, "ARRAYS\n");
  for (i = 0; i < c->arrays->size; i++) {
    array_info *cur = vector_get_array_info(c->arrays, i);
    strcat(result, cur->name);
    strcat(result, "\n");
  }

  strcat(result, "STRUCTS\n");
  for (i = 0; i < c->structs->size; i++) {
    struct_info *cur = vector_get_struct_info(c->structs, i);
    strcat(result, cur->name);
    strcat(result, "\n");
  }

  strcat(result, "VARS\n");
  for (i = 0; i < c->vars->size; i++) {
    var_info *cur = vector_get_var_info(c->vars, i);
    strcat(result, cur->name);
    strcat(result, "\n");
  }
  return result;
}
