#include "c_ir.h"
#include "compiler_error.h"
#include "safe.h"
#include "vector.h"
#include "vector_get.h"
#include <stdio.h>

c_prog *gen_c_ir(vector *cmds, ctx *ctx) {
  // Setup program env
  c_prog *program = safe_alloc(sizeof(c_prog));
  program->ctx = ctx;
  program->fns = safe_alloc(sizeof(vector));
  program->structs = safe_alloc(sizeof(c_struct));
  vector_init(program->fns, 8, CFNVECTOR);
  vector_init(program->structs, 8, CSTRUCTVECTOR);

  // Setup jpl_main
  c_fn *jpl_main = safe_alloc(sizeof(c_fn));
  jpl_main->name = safe_alloc(9);
  memcpy(jpl_main->name, "jpl_main", 8);
  jpl_main->parent = program;

  return program;
}

char *fn_gensym(c_fn *fn) {
  char *result = safe_alloc(1);
  result = safe_strcat(result, "_");

  char *num = safe_alloc(BUFSIZ);
  sprintf(num, "%d", fn->name_ctr);
  fn->name_ctr += 1;

  result = safe_strcat(result, num);
  free(num);

  return result;
}
