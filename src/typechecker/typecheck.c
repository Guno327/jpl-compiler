#include "typecheck.h"
#include "alloc.h"
#include "compiler_error.h"
#include "ctx.h"
#include "t.h"

void typecheck(vector *program) {
  // Setup global ctx
  ctx *global = alloc(sizeof(ctx));
  global->structs = alloc(sizeof(vector));
  vector_init(global->structs, 8, STRUCTINFOVECTOR);

  struct_info *rgba = alloc(sizeof(struct_info));
  rgba->name = "rgba";
  rgba->vars = alloc(sizeof(vector));
  rgba->ts = alloc(sizeof(vector));
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

  // Loop through cmds
  global->parent = NULL;
  for (int i = 0; i < program->size; i++) {
  }

  return;
}
