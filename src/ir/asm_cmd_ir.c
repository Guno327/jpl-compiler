#include "asm_ir.h"
#include "compiler_error.h"
#include "safe.h"
#include "vector.h"
#include "vector_get.h"
#include <stdio.h>

void cmd_asmgen(asm_prog *prog, asm_fn *fn, cmd *c) {
  switch (c->type) {
  case SHOWCMD:;
    show_cmd *sc = (show_cmd *)c->node;

  default:;
    char *msg = safe_alloc(BUFSIZ);
    sprintf(msg, "CMD of type %d not implemented yet", c->type);
    ir_error(msg);
  }
}
