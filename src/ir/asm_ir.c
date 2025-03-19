#include "asm_ir.h"
#include "safe.h"
#include "vector.h"
#include "vector_get.h"
#include <string.h>

asm_prog *gen_asm_ir(vector *cmds, ctx *ctx) {
  // Setup prog
  asm_prog *prog = safe_alloc(sizeof(asm_prog));
  prog->ctx = ctx;
  prog->fns = safe_alloc(sizeof(vector));
  prog->stk = safe_alloc(sizeof(stack));
  prog->data = safe_alloc(sizeof(vector));
  prog->const_names = safe_alloc(sizeof(vector));
  prog->const_vals = safe_alloc(sizeof(vector));
  prog->jmp_ctr = 0;

  vector_init(prog->fns, 8, ASMFNVECTOR);
  vector_init(prog->data, 8, STRVECTOR);
  vector_init(prog->const_names, 8, STRVECTOR);
  vector_init(prog->const_vals, 8, STRVECTOR);

  // Setup jpl_main
  asm_fn *jpl_main = safe_alloc(sizeof(asm_fn));
  jpl_main->stk = safe_alloc(sizeof(stack));
  jpl_main->code = safe_alloc(sizeof(vector));
  jpl_main->name = safe_alloc(9);
  memcpy(jpl_main->name, "jpl_main", 8);
  vector_append(prog->fns, jpl_main);

  // Process cmds
  for (int i = 0; i < cmds->size; i++) {
    cmd *c = vector_get_cmd(cmds, i);
    cmd_asmgen(prog, jpl_main, c);
  }

  return prog;
}
