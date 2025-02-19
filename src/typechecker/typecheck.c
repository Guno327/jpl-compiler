#include "typecheck.h"
#include "alloc.h"
#include "compiler_error.h"
#include "ctx.h"
#include "t.h"
#include "vector_get.h"
#include <stdio.h>
#include <string.h>

void typecheck(vector *program) {
  // Setup global ctx
  ctx *global = setup_global_ctx();

  // Loop through cmds
  global->parent = NULL;
  for (int i = 0; i < program->size; i++) {
    cmd *cc = vector_get_cmd(program, i);
    type_cmd(cc, global);
  }
}

ctx *setup_global_ctx() {
  ctx *c = alloc(sizeof(ctx));
  c->structs = alloc(sizeof(vector));
  c->arrays = alloc(sizeof(vector));
  c->fns = alloc(sizeof(vector));

  vector_init(c->structs, 8, STRUCTINFOVECTOR);
  vector_init(c->arrays, 8, ARRAYINFOVECTOR);
  vector_init(c->fns, 8, FNINFOVECTOR);

  // rgba
  struct_info *rgba = alloc(sizeof(struct_info));
  rgba->name = "rgba";
  rgba->ts = alloc(sizeof(vector));
  rgba->vars = alloc(sizeof(vector));
  vector_init(rgba->vars, 4, STRVECTOR);
  vector_init(rgba->ts, 4, TVECTOR);

  vector_append(rgba->vars, "r");
  vector_append(rgba->vars, "g");
  vector_append(rgba->vars, "b");
  vector_append(rgba->vars, "a");

  for (int i = 0; i < 4; i++) {
    t *float_t = alloc(sizeof(t));
    float_t->type = FLOAT_T;
    vector_append(rgba->ts, float_t);
  }
  vector_append(c->structs, rgba);

  // One-float built-ins
  t *f = alloc(sizeof(t));
  f->type = FLOAT_T;
  f->info = NULL;

  fn_info *f_info = alloc(sizeof(fn_info));
  f_info->ret = f;
  f_info->args = alloc(sizeof(vector));
  vector_init(f_info->args, 1, TVECTOR);
  vector_append(f_info->args, f);

  // sqrt
  fn_info *sqrt_info = alloc(sizeof(fn_info));
  memcpy(sqrt_info, f_info, sizeof(fn_info));
  sqrt_info->name = "sqrt";
  vector_append(c->fns, sqrt_info);

  // exp
  fn_info *exp_info = alloc(sizeof(fn_info));
  memcpy(exp_info, f_info, sizeof(fn_info));
  exp_info->name = "exp";
  vector_append(c->fns, exp_info);

  // sin
  fn_info *sin_info = alloc(sizeof(fn_info));
  memcpy(sin_info, f_info, sizeof(fn_info));
  sin_info->name = "sin";
  vector_append(c->fns, sin_info);

  // tan
  fn_info *tan_info = alloc(sizeof(fn_info));
  memcpy(tan_info, f_info, sizeof(fn_info));
  tan_info->name = "tan";
  vector_append(c->fns, tan_info);

  // asin
  fn_info *asin_info = alloc(sizeof(fn_info));
  memcpy(asin_info, f_info, sizeof(fn_info));
  asin_info->name = "asin";
  vector_append(c->fns, asin_info);

  // acos
  fn_info *acos_info = alloc(sizeof(fn_info));
  memcpy(acos_info, f_info, sizeof(fn_info));
  acos_info->name = "acos";
  vector_append(c->fns, acos_info);

  // atan
  fn_info *atan_info = alloc(sizeof(fn_info));
  memcpy(atan_info, f_info, sizeof(fn_info));
  atan_info->name = "atan";
  vector_append(c->fns, atan_info);

  // log
  fn_info *log_info = alloc(sizeof(fn_info));
  memcpy(log_info, f_info, sizeof(fn_info));
  log_info->name = "log";
  vector_append(c->fns, log_info);
  free(f_info);

  // Two-float built-ints
  fn_info *ff_info = alloc(sizeof(fn_info));
  ff_info->ret = f;
  ff_info->args = alloc(sizeof(vector));
  vector_init(ff_info->args, 2, TVECTOR);
  vector_append(ff_info->args, f);
  vector_append(ff_info->args, f);

  // pow
  fn_info *pow_info = alloc(sizeof(fn_info));
  memcpy(pow_info, ff_info, sizeof(fn_info));
  pow_info->name = "pow";
  vector_append(c->fns, pow_info);

  // atan2
  fn_info *atan2_info = alloc(sizeof(fn_info));
  memcpy(atan2_info, ff_info, sizeof(fn_info));
  atan2_info->name = "atan2";
  vector_append(c->fns, atan2_info);
  free(ff_info);

  // Conversion functions
  t *i = alloc(sizeof(t));
  i->type = INT_T;
  i->info = NULL;

  // to_float
  fn_info *to_float_info = alloc(sizeof(fn_info));
  to_float_info->name = "to_float";
  to_float_info->ret = f;
  to_float_info->args = alloc(sizeof(vector));
  vector_init(to_float_info->args, 1, TVECTOR);
  vector_append(to_float_info->args, i);
  vector_append(c->fns, to_float_info);

  // to_int
  fn_info *to_int_info = alloc(sizeof(fn_info));
  to_int_info->name = "to_int";
  to_int_info->ret = i;
  to_int_info->args = alloc(sizeof(vector));
  vector_init(to_int_info->args, 1, TVECTOR);
  vector_append(to_int_info->args, f);
  vector_append(c->fns, to_int_info);

  return c;
}
