#include "c_ir.h"
#include "compiler_error.h"
#include "safe.h"
#include "typecheck.h"
#include "vector.h"
#include "vector_get.h"
#include <stdio.h>
#include <string.h>

c_prog *gen_c_ir(vector *cmds, ctx *ctx) {
  // Setup program env
  c_prog *program = safe_alloc(sizeof(c_prog));
  program->ctx = ctx;
  program->jump_ctr = 1;
  program->fns = safe_alloc(sizeof(vector));
  program->structs = safe_alloc(sizeof(c_struct));
  vector_init(program->fns, 8, CFNVECTOR);
  vector_init(program->structs, 8, CSTRUCTVECTOR);

  // Setup jpl_main
  c_fn *jpl_main = safe_alloc(sizeof(c_fn));
  jpl_main->ret_type = safe_alloc(5);
  memcpy(jpl_main->ret_type, "void", 4);
  jpl_main->name = safe_alloc(9);
  memcpy(jpl_main->name, "jpl_main", 8);
  jpl_main->args_list = safe_alloc(1);
  jpl_main->parent = NULL;
  jpl_main->name_ctr = 0;
  jpl_main->code = safe_alloc(sizeof(vector));
  jpl_main->c_names = safe_alloc(sizeof(vector));
  jpl_main->jpl_names = safe_alloc(sizeof(vector));
  vector_init(jpl_main->code, 8, STRVECTOR);
  vector_init(jpl_main->c_names, 8, STRVECTOR);
  vector_init(jpl_main->jpl_names, 8, STRVECTOR);
  vector_append(program->fns, jpl_main);

  // Handle cmds
  for (int i = 0; i < cmds->size; i++) {
    cmd *cur_cmd = vector_get_cmd(cmds, i);
    cmd_gencode(program, jpl_main, cur_cmd);
  }

  return program;
}

char *gensym(c_fn *fn) {
  char *result = safe_alloc(1);
  result = safe_strcat(result, "_");

  char *num = safe_alloc(BUFSIZ);
  sprintf(num, "%d", fn->name_ctr);
  fn->name_ctr += 1;

  result = safe_strcat(result, num);
  free(num);

  return result;
}

char *genjmp(c_prog *prog) {
  char *result = safe_alloc(1);
  result = safe_strcat(result, "_jump");

  char *num = safe_alloc(BUFSIZ);
  sprintf(num, "%d", prog->jump_ctr);
  prog->jump_ctr += 1;

  result = safe_strcat(result, num);
  free(num);

  return result;
}

char *genarray(c_prog *prog, c_fn *fn, t *type, int rank) {
  char *result = safe_alloc(1);
  result = safe_strcat(result, "_a");
  char *rank_str = safe_alloc(BUFSIZ);
  sprintf(rank_str, "%d", rank);
  result = safe_strcat(result, rank_str);
  free(rank_str);
  result = safe_strcat(result, "_");

  switch (type->type) {
  case INT_T:
    result = safe_strcat(result, "int64_t");
    break;
  case FLOAT_T:
    result = safe_strcat(result, "double");
    break;
  case VOID_T:
    result = safe_strcat(result, "void_t");
    break;
  case BOOL_T:
    result = safe_strcat(result, "bool");
    break;
  case ARRAY_T:;
    array_info *a_info = (array_info *)type->info;
    char *inner = genarray(prog, fn, a_info->type, rank);
    result = safe_strcat(result, inner);
    break;
  case STRUCT_T:;
    struct_info *s_info = (struct_info *)type->info;
    result = safe_strcat(result, s_info->name);
    break;
  case FN_T:;
    char *msg = safe_alloc(BUFSIZ);
    sprintf(msg, "functions are not first class");
    ir_error(msg);
  }

  // Check if we need to add struct decleration
  bool found = false;
  for (int i = 0; i < prog->structs->size; i++) {
    c_struct *cur = vector_get_c_struct(prog->structs, i);
    if (!strcmp(cur->name, result)) {
      found = true;
      break;
    }
  }
  // Add struct decleration
  if (!found) {
    char *int_t = safe_alloc(BUFSIZ);
    sprintf(int_t, "int64_t");

    c_struct *s = safe_alloc(sizeof(c_struct));
    s->name = result;
    s->types = safe_alloc(sizeof(vector));
    s->fields = safe_alloc(sizeof(vector));
    vector_init(s->types, rank + 1, STRVECTOR);
    vector_init(s->fields, rank + 1, STRVECTOR);

    for (int i = 0; i < rank; i++) {
      char *cur_d = safe_alloc(BUFSIZ);
      sprintf(cur_d, "d%d", i);
      vector_append(s->fields, cur_d);
      vector_append(s->types, int_t);
    }

    char *data = safe_alloc(BUFSIZ);
    sprintf(data, "*data");
    char *data_t = gent(prog, fn, type);
    vector_append(s->fields, data);
    vector_append(s->types, data_t);

    vector_append(prog->structs, s);
  }

  return result;
}

char *jpl_to_c(c_fn *fn, char *jpl_name) {
  while (fn != NULL) {
    for (int i = 0; i < fn->jpl_names->size; i++) {
      if (!strcmp(jpl_name, vector_get_str(fn->jpl_names, i))) {
        return vector_get_str(fn->c_names, i);
      }
    }
    fn = fn->parent;
  }

  char *msg = safe_alloc(BUFSIZ);
  sprintf(msg, "Could not map jpl_name '%s' to c_names", jpl_name);
  ir_error(msg);
  // Should not return, but needed for syntax highlightning
  return NULL;
}

char *gent(c_prog *prog, c_fn *fn, t *t) {
  char *result = safe_alloc(1);
  switch (t->type) {
  case INT_T:
    result = safe_strcat(result, "int64_t");
    break;
  case FLOAT_T:
    result = safe_strcat(result, "double");
    break;
  case BOOL_T:
    result = safe_strcat(result, "bool");
    break;
  case VOID_T:
    result = safe_strcat(result, "void_t");
    break;
  case STRUCT_T:;
    struct_info *s_info = (struct_info *)t->info;
    result = safe_strcat(result, s_info->name);
    break;
  case ARRAY_T:;
    array_info *a_info = (array_info *)t->info;
    char *c_name = genarray(prog, fn, a_info->type, a_info->rank);
    result = safe_strcat(result, c_name);
    break;
  case FN_T:;
    char *msg = safe_alloc(BUFSIZ);
    sprintf(msg, "functions are not first class");
    ir_error(msg);
  }
  return result;
}
char *genshowt(t *t) {
  if (t == NULL)
    return NULL;

  char *result = safe_alloc(BUFSIZ);
  switch (t->type) {
  case INT_T:
    sprintf(result, "(IntType)");
    break;
  case FLOAT_T:
    sprintf(result, "(FloatType)");
    break;
  case BOOL_T:
    sprintf(result, "(BoolType)");
    break;
  case VOID_T:
    sprintf(result, "(VoidType)");
    break;
  case ARRAY_T:;
    array_info *a_info = (array_info *)t->info;
    char *a_type = genshowt(a_info->type);
    result = safe_realloc_str(result, BUFSIZ + 1 + strlen(result) +
                                          strlen(a_type) + 12);
    sprintf(result, "(ArrayType %s %d)", a_type, a_info->rank);
    free(a_type);
    break;
  case STRUCT_T:;
    struct_info *s_info = (struct_info *)t->info;
    result = safe_strcat(result, "(TupleType ");

    for (int i = 0; i < s_info->ts->size; i++) {
      char *cur_t = genshowt(vector_get_t(s_info->ts, i));
      result = safe_strcat(result, cur_t);
      if (i != s_info->ts->size - 1)
        result = safe_strcat(result, " ");
      free(cur_t);
    }
    result = safe_strcat(result, ")");
    break;
  case FN_T:
    // TODO HW7
    sprintf(result, "(FnType)");
    break;
  default:
    return NULL;
  }

  return result;
}
