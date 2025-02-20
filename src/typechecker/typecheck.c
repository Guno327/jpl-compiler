#include "typecheck.h"
#include "compiler_error.h"
#include "ctx.h"
#include "safe.h"
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
  ctx *c = setup_ctx();

  vector_init(c->structs, 8, STRUCTINFOVECTOR);
  vector_init(c->arrays, 8, ARRAYINFOVECTOR);
  vector_init(c->fns, 8, FNINFOVECTOR);

  // rgba
  struct_info *rgba = safe_alloc(sizeof(struct_info));
  rgba->name = "rgba";
  rgba->ts = safe_alloc(sizeof(vector));
  rgba->vars = safe_alloc(sizeof(vector));
  vector_init(rgba->vars, 4, STRVECTOR);
  vector_init(rgba->ts, 4, TVECTOR);

  vector_append(rgba->vars, "r");
  vector_append(rgba->vars, "g");
  vector_append(rgba->vars, "b");
  vector_append(rgba->vars, "a");

  for (int i = 0; i < 4; i++) {
    t *float_t = safe_alloc(sizeof(t));
    float_t->type = FLOAT_T;
    vector_append(rgba->ts, float_t);
  }
  vector_append(c->structs, rgba);

  // Types we need for built-ins
  t *i = safe_alloc(sizeof(t));
  i->type = INT_T;
  i->info = NULL;

  t *f = safe_alloc(sizeof(t));
  f->type = FLOAT_T;
  f->info = NULL;

  // Built-in vars
  // argnum
  var_info *argc = safe_alloc(sizeof(var_info));
  argc->t = i;
  argc->name = "argnum";
  vector_append(c->vars, argc);

  // args
  array_info *argv = safe_alloc(sizeof(array_info));
  argv->type = i;
  argv->rank = 1;
  argv->name = "args";
  vector_append(c->arrays, argv);

  // One-float built-ins
  fn_info *f_info = safe_alloc(sizeof(fn_info));
  f_info->ret = f;
  f_info->args = safe_alloc(sizeof(vector));
  vector_init(f_info->args, 1, BINDINGVECTOR);
  binding *a = safe_alloc(sizeof(binding));
  a->lval = safe_alloc(sizeof(lval));
  a->lval->type = VARLVALUE;
  a->lval->node = safe_alloc(sizeof(var_lval));
  ((var_lval *)a->lval->node)->var = "a";
  a->type = safe_alloc(sizeof(type));
  a->type->type = FLOATTYPE;
  a->type->node = safe_alloc(sizeof(float_type));
  vector_append(f_info->args, a);

  // sqrt
  fn_info *sqrt_info = safe_alloc(sizeof(fn_info));
  memcpy(sqrt_info, f_info, sizeof(fn_info));
  sqrt_info->name = "sqrt";
  vector_append(c->fns, sqrt_info);

  // exp
  fn_info *exp_info = safe_alloc(sizeof(fn_info));
  memcpy(exp_info, f_info, sizeof(fn_info));
  exp_info->name = "exp";
  vector_append(c->fns, exp_info);

  // sin
  fn_info *sin_info = safe_alloc(sizeof(fn_info));
  memcpy(sin_info, f_info, sizeof(fn_info));
  sin_info->name = "sin";
  vector_append(c->fns, sin_info);

  // cos
  fn_info *cos_info = safe_alloc(sizeof(fn_info));
  memcpy(cos_info, f_info, sizeof(fn_info));
  cos_info->name = "cos";
  vector_append(c->fns, cos_info);

  // tan
  fn_info *tan_info = safe_alloc(sizeof(fn_info));
  memcpy(tan_info, f_info, sizeof(fn_info));
  tan_info->name = "tan";
  vector_append(c->fns, tan_info);

  // asin
  fn_info *asin_info = safe_alloc(sizeof(fn_info));
  memcpy(asin_info, f_info, sizeof(fn_info));
  asin_info->name = "asin";
  vector_append(c->fns, asin_info);

  // acos
  fn_info *acos_info = safe_alloc(sizeof(fn_info));
  memcpy(acos_info, f_info, sizeof(fn_info));
  acos_info->name = "acos";
  vector_append(c->fns, acos_info);

  // atan
  fn_info *atan_info = safe_alloc(sizeof(fn_info));
  memcpy(atan_info, f_info, sizeof(fn_info));
  atan_info->name = "atan";
  vector_append(c->fns, atan_info);

  // log
  fn_info *log_info = safe_alloc(sizeof(fn_info));
  memcpy(log_info, f_info, sizeof(fn_info));
  log_info->name = "log";
  vector_append(c->fns, log_info);

  // Two-float built-ints
  fn_info *ff_info = safe_alloc(sizeof(fn_info));
  ff_info->ret = f;
  ff_info->args = safe_alloc(sizeof(vector));
  vector_init(ff_info->args, 2, BINDINGVECTOR);
  binding *b = safe_alloc(sizeof(binding));
  b->lval = safe_alloc(sizeof(lval));
  b->lval->type = VARLVALUE;
  b->lval->node = safe_alloc(sizeof(var_lval));
  ((var_lval *)b->lval->node)->var = "b";
  b->type = safe_alloc(sizeof(type));
  b->type->type = FLOATTYPE;
  b->type->node = safe_alloc(sizeof(float_type));
  vector_append(ff_info->args, a);
  vector_append(ff_info->args, b);

  // pow
  fn_info *pow_info = safe_alloc(sizeof(fn_info));
  memcpy(pow_info, ff_info, sizeof(fn_info));
  pow_info->name = "pow";
  vector_append(c->fns, pow_info);

  // atan2
  fn_info *atan2_info = safe_alloc(sizeof(fn_info));
  memcpy(atan2_info, ff_info, sizeof(fn_info));
  atan2_info->name = "atan2";
  vector_append(c->fns, atan2_info);

  // Conversion functions
  // to_float
  fn_info *to_float_info = safe_alloc(sizeof(fn_info));
  to_float_info->name = "to_float";
  to_float_info->ret = f;
  to_float_info->args = safe_alloc(sizeof(vector));

  vector_init(to_float_info->args, 1, BINDINGVECTOR);
  binding *ia = safe_alloc(sizeof(binding));
  ia->lval = safe_alloc(sizeof(lval));
  ia->lval->type = VARLVALUE;
  ia->lval->node = safe_alloc(sizeof(var_lval));
  ((var_lval *)ia->lval->node)->var = "a";
  ia->type = safe_alloc(sizeof(type));
  ia->type->type = INTTYPE;
  ia->type->node = safe_alloc(sizeof(INTTYPE));
  vector_append(to_float_info->args, ia);
  vector_append(c->fns, to_float_info);

  // to_int
  fn_info *to_int_info = safe_alloc(sizeof(fn_info));
  to_int_info->name = "to_int";
  to_int_info->ret = i;
  to_int_info->args = safe_alloc(sizeof(vector));

  vector_init(to_int_info->args, 1, BINDINGVECTOR);
  vector_append(to_int_info->args, a);
  vector_append(c->fns, to_int_info);

  return c;
}
