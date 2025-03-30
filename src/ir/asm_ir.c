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
  prog->jmp_ctr = 1;

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
  jpl_main->stk->shadow = safe_alloc(sizeof(vector));
  jpl_main->stk->names = safe_alloc(sizeof(vector));
  jpl_main->stk->positions = safe_alloc(sizeof(vector));
  vector_init(jpl_main->stk->shadow, 8, TVECTOR);
  vector_init(jpl_main->stk->names, 8, STRVECTOR);
  vector_init(jpl_main->stk->positions, 8, NUMVECTOR);

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
    char *code = safe_alloc(BUFSIZ);
    sprintf(code, "add rsp, %lu\n", jpl_main->stk->size - 8);
    vector_append(jpl_main->code, code);
  }
  vector_append(jpl_main->code, "pop r12\npop rbp\nret\n");

  return prog;
}

void stack_push(asm_fn *fn, char *reg) {
  t *placeholder = safe_alloc(sizeof(t));
  placeholder->type = INT_T;
  placeholder->info = NULL;
  vector_append(fn->stk->shadow, placeholder);

  char *code = safe_alloc(BUFSIZ);
  if (!strncmp(reg, "xmm", 3))
    sprintf(code, "sub rsp, 8\nmovsd [rsp], %s\n", reg);
  else
    sprintf(code, "push %s\n", reg);
  vector_append(fn->code, code);
}

void stack_rechar(asm_fn *fn, t *type, size_t size) {
  for (int i = 0; i < size; i++) {
    t *cur = vector_get_t(fn->stk->shadow, fn->stk->shadow->size - i - 1);
    free(cur);
  }
  fn->stk->shadow->size -= size;

  fn->stk->size += sizeof_t(type);
  vector_append(fn->stk->shadow, type);
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
  size_t leftovers = (fn->stk->size + size) % 16;
  if (leftovers != 0)
    leftovers = 16 - leftovers;
  padding *pad = safe_alloc(sizeof(padding));
  pad->size = leftovers;

  t *padding = safe_alloc(sizeof(t));
  padding->type = PAD_T;
  padding->info = pad;

  vector_append(fn->stk->shadow, padding);
  fn->stk->size += leftovers;

  if (leftovers > 0) {
    char *cmd = safe_alloc(BUFSIZ);
    sprintf(cmd, "sub rsp, %lu\n", leftovers);
    vector_append(fn->code, cmd);
  }
}

void stack_unalign(asm_fn *fn) {
  t *pad = vector_get_t(fn->stk->shadow, fn->stk->shadow->size - 1);
  fn->stk->shadow->size -= 1;
  if (pad == NULL)
    ir_error("Tried to unalign empty stack");
  if (pad->type != PAD_T)
    ir_error("Stack unalign error: not padding");

  padding *info = (padding *)pad->info;
  size_t size = info->size;
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

  vector_append(fn->code, assert_code);
  assert_code = safe_alloc(1);
  stack_unalign(fn);

  assert_code = safe_strcat(assert_code, jmp);
  assert_code = safe_strcat(assert_code, ":\n");
  vector_append(fn->code, assert_code);
}

void push_lval(asm_fn *fn, lval *lval, size_t base) {
  switch (lval->type) {
  case VARLVALUE:;
    var_lval *vlv = (var_lval *)lval->node;
    vector_append(fn->stk->names, vlv->var);
    vector_append(fn->stk->positions, (void *)base);
    break;
  case ARRAYLVALUE:;
    array_lval *alv = (array_lval *)lval->node;
    vector_append(fn->stk->names, alv->var);
    vector_append(fn->stk->positions, (void *)base);

    for (int i = 0; i < alv->vars->size; i++) {
      char *cur_name = vector_get_str(alv->vars, i);
      vector_append(fn->stk->names, cur_name);
      vector_append(fn->stk->positions, (void *)base);
      base -= 8;
    }
    break;
  }
}

void let_asmgen(asm_prog *prog, asm_fn *fn, void *let, bool is_stmt) {
  lval *lv = NULL;
  expr *e = NULL;

  if (is_stmt) {
    let_stmt *ls = (let_stmt *)let;
    lv = ls->lval;
    e = ls->expr;
  } else {
    let_cmd *lc = (let_cmd *)let;
    lv = lc->lval;
    e = lc->expr;
  }

  expr_asmgen(prog, fn, e);
  push_lval(fn, lv, fn->stk->size);
}

void stack_alloc(asm_fn *fn, t *type) {
  size_t type_size = sizeof_t(type);

  vector_append(fn->stk->shadow, type);
  fn->stk->size += type_size;

  char *code = safe_alloc(BUFSIZ);
  sprintf(code, "sub rsp, %lu\n", type_size);
  vector_append(fn->code, code);
}

void stack_copy(asm_fn *fn, t *type, char *start, char *end) {
  size_t size = sizeof_t(type);
  for (long offset = size - 8; offset >= 0; offset -= 8) {
    char *code = safe_alloc(BUFSIZ);
    sprintf(code, "mov r10, [%s + %lu]\nmov [%s + %lu], r10\n", start, offset,
            end, offset);
    vector_append(fn->code, code);
  }
}

size_t stack_lookup(stack *stk, char *var) {
  size_t result = -1;
  for (int i = 0; i < stk->names->size; i++) {
    char *cur = vector_get_str(stk->names, i);
    if (!strcmp(cur, var)) {
      result = vector_get_num(stk->positions, i);
      break;
    }
  }
  return result;
}
