#include "typecheck.h"
#include "alloc.h"
#include "compiler_error.h"
#include "ctx.h"
#include "t.h"

void typecheck(vector *program) {
  // Setup global ctx
  ctx *global = alloc(sizeof(ctx));
  global->parent = NULL;
  for (int i = 0; i < program->size; i++) {
  }

  return;
}
