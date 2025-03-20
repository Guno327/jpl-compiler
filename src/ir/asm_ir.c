#include "asm_ir.h"
#include "safe.h"
#include "vector.h"
#include "vector_get.h"
#include <stdio.h>
#include <string.h>

asm_prog *gen_asm_ir(vector *cmds, ctx *ctx) {
  // Setup prog
  asm_prog *prog = safe_alloc(sizeof(asm_prog));
  prog->ctx = ctx;
  prog->fns = safe_alloc(sizeof(vector));
  prog->data = safe_alloc(sizeof(vector));
  prog->const_names = safe_alloc(sizeof(vector));
  prog->const_vals = safe_alloc(sizeof(vector));
  prog->jmp_ctr = 0;

  vector_init(prog->fns, 8, ASMFNVECTOR);
  vector_init(prog->data, 8, STRVECTOR);
  vector_init(prog->const_names, 8, STRVECTOR);
  vector_init(prog->const_vals, 8, STRVECTOR);

  prog->stk = safe_alloc(sizeof(stack));

  // Setup jpl_main
  asm_fn *jpl_main = safe_alloc(sizeof(asm_fn));
  jpl_main->name = safe_alloc(9);
  memcpy(jpl_main->name, "jpl_main", 8);

  jpl_main->stk = prog->stk;
  jpl_main->stk->fn = jpl_main;
  jpl_main->stk->size = -16;
  jpl_main->stk->shadow = safe_alloc(sizeof(vector));
  vector_init(jpl_main->stk->shadow, 8, TVECTOR);

  t *int_t = safe_alloc(sizeof(t));
  int_t->type = INT_T;
  int_t->info = NULL;
  vector_append(jpl_main->stk->shadow, int_t);
  jpl_main->stk->size = 8;

  jpl_main->code = safe_alloc(sizeof(vector));
  vector_init(jpl_main->code, 8, STRVECTOR);
  vector_append(jpl_main->code, "push rbp ; mov rbp ; rsp ;\n");
  vector_append(jpl_main->code, "push r12 ; move r12, rsp\n");
  vector_append(prog->fns, jpl_main);

  // Process cmds
  for (int i = 0; i < cmds->size; i++) {
    cmd *c = vector_get_cmd(cmds, i);
    cmd_asmgen(prog, jpl_main, c);
  }

  if (jpl_main->stk->size > 8) {
    char *line = safe_alloc(BUFSIZ);
    sprintf(line, "add rsp, (%lu - 8)\n", jpl_main->stk->size);
    vector_append(jpl_main->code, line);
  }

  return prog;
}

void stack_push(asm_fn *fn, char *reg, t *type) {
  size_t t_size = sizeof_t(type);
  fn->stk->size += t_size;
  vector_append(fn->stk->shadow, type);

  char *code = safe_alloc(BUFSIZ);
  sprintf(code, "push %s\n", reg);
  vector_append(fn->code, code);
}

t *stack_pop(asm_fn *fn) {
  fn->stk->shadow->size -= 1;
  t *item = vector_get_t(fn->stk->shadow, fn->stk->shadow->size);
  size_t t_size = sizeof_t(item);
  fn->stk->fn->stk->size -= t_size;

  char *code = safe_alloc(BUFSIZ);
  sprintf(code, "add rsp, %lu\n", t_size);
  vector_append(fn->code, code);

  return item;
}

char *genconst(asm_prog *prog, char *val) {
  char *result = NULL;
  for (int i = 0; i < prog->const_vals->size; i++) {
    char *cur = vector_get_str(prog->const_vals, i);
    if (!strcmp(cur, val)) {
      result = vector_get_str(prog->const_names, i);
      break;
    }
  }

  if (result == NULL) {
    result = safe_alloc(BUFSIZ);
    sprintf(result, "const%d", prog->const_vals->size);
    vector_append(prog->const_names, result);
    vector_append(prog->const_vals, val);
  }

  return result;
}

void stack_align(asm_fn *fn, size_t amount) {
  if (amount == 0)
    return;

  char *code = safe_alloc(BUFSIZ);
  sprintf(code, "sub rsp, %lu\n", amount);
  vector_append(fn->code, code);
}

void stack_unalign(asm_fn *fn, size_t amount) {
  if (amount == 0)
    return;

  char *code = safe_alloc(BUFSIZ);
  sprintf(code, "add rsp, %lu\n", amount);
  vector_append(fn->code, code);
}
