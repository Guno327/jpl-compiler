#include "asm_ir.h"
#include "compiler_error.h"
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
  vector_append(jpl_main->code, "push rbp\nmov rbp, rsp\n");
  vector_append(jpl_main->code, "push r12\nmov r12, rbp\n");
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

  if (jpl_main->stk->size > 8) {
    vector_append(jpl_main->code, "add rsp, (size-8)\n");
  }
  vector_append(jpl_main->code, "pop r12\npop rbp\nret\n");

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

t *stack_pop(asm_fn *fn, char *reg) {
  t *item = vector_get_t(fn->stk->shadow, fn->stk->shadow->size - 1);
  fn->stk->shadow->size -= 1;
  size_t t_size = sizeof_t(item);
  fn->stk->fn->stk->size -= t_size;

  char *code = safe_alloc(BUFSIZ);
  if (reg == NULL)
    sprintf(code, "add rsp, %lu\n", t_size);
  else if (item->type == FLOAT_T)
    sprintf(code, "movsd %s, [rsp]\nadd rsp, 8\n", reg);
  else
    sprintf(code, "pop %s\n", reg);
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

void stack_align(asm_fn *fn, size_t size) {
  size = (fn->stk->size + size) % 16;
  padding *pad = safe_alloc(sizeof(padding));
  pad->size = size;

  t *padding = safe_alloc(sizeof(t));
  padding->type = PAD_T;
  padding->info = pad;

  vector_append(fn->stk->shadow, padding);
  fn->stk->size += size;

  if (size > 0) {
    char *cmd = safe_alloc(BUFSIZ);
    sprintf(cmd, "add rsp, %lu", size);
    vector_append(fn->code, cmd);
  }
}

void stack_unalign(asm_fn *fn) {
  t *pad = vector_get_t(fn->stk->shadow, fn->stk->shadow->size - 1);
  if (pad == NULL)
    ir_error("Tried to unalign empty stack");
  if (pad->type != PAD_T)
    ir_error("Stack unalign error: not padding");

  size_t size = ((padding *)pad->info)->size;
  fn->stk->size -= size;
  if (size > 0) {
    char *cmd = safe_alloc(BUFSIZ);
    sprintf(cmd, "add rsp, %lu\n", size);
    vector_append(fn->code, cmd);
  }
}

void assert_asmgen(asm_prog *prog, asm_fn *fn, char *msg) {
  char *jmp = safe_alloc(BUFSIZ);
  sprintf(jmp, ".jump%d", prog->jmp_ctr);
  prog->jmp_ctr += 1;

  char *assert_code = safe_alloc(1);
  assert_code = safe_strcat(assert_code, "jne ");
  assert_code = safe_strcat(assert_code, jmp);
  assert_code = safe_strcat(assert_code, "\n");
  vector_append(fn->code, assert_code);
  assert_code = safe_alloc(1);

  stack_align(fn, 0);

  char *assert_var = safe_alloc(BUFSIZ + strlen(msg));
  sprintf(assert_var, "db `%s`, 0", msg);
  char *assert_const = genconst(prog, assert_var);

  assert_code = safe_strcat(assert_code, "lea rdi, [rel ");
  assert_code = safe_strcat(assert_code, assert_const);
  assert_code = safe_strcat(assert_code, "]\n");

  assert_code = safe_strcat(assert_code, "call _fail_assertion\n");
  stack_unalign(fn);

  assert_code = safe_strcat(assert_code, jmp);
  assert_code = safe_strcat(assert_code, ":\n");

  vector_append(fn->code, assert_code);
}
