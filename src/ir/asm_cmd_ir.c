#include "asm_ir.h"
#include "compiler_error.h"
#include "safe.h"
#include "vector.h"
#include "vector_get.h"
#include <stdio.h>
#include <string.h>

void cmd_asmgen(asm_prog *prog, asm_fn *fn, cmd *c) {
  switch (c->type) {
  case SHOWCMD:;
    show_cmd *sc = (show_cmd *)c->node;
    stack_align(fn, sizeof_t(sc->expr->t_type) % 16);
    expr_asmgen(prog, fn, sc->expr);

    char *sc_type = t_to_str(sc->expr->t_type);
    char *sc_val = safe_alloc(strlen(sc_type) + BUFSIZ);
    sprintf(sc_val, "db `%s`, 0", sc_type);
    free(sc_type);
    char *sc_const = genconst(prog, sc_val);

    char *sc_code = safe_alloc(1);
    sc_code = safe_strcat(sc_code, "lea rdi, [rel ");
    sc_code = safe_strcat(sc_code, sc_const);
    sc_code = safe_strcat(sc_code, "]\n");

    sc_code = safe_strcat(sc_code, "lea rsi, [rsp]\n");
    sc_code = safe_strcat(sc_code, "call show\n");
    vector_append(fn->code, sc_code);

    t *sc_pop = stack_pop(fn, NULL);
    if (!t_eq(sc_pop, sc->expr->t_type))
      ir_error("Stack Error in SHOWCMD");
    stack_unalign(fn, sizeof_t(sc->expr->t_type) % 16);
    break;
  default:;
    char *msg = safe_alloc(BUFSIZ);
    sprintf(msg, "CMD of type %d not implemented yet", c->type);
    ir_error(msg);
  }
}
