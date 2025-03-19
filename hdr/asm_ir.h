#include "ast.h"
#include "ctx.h"
#include "t.h"
#include "vector.h"
#ifndef ASM_IR_H
#define ASM_IR_H

struct asm_prog;
struct asm_fn;
struct stack;

typedef struct stack {
  struct asm_fn *fn;
  int size;
  vector *names;
  vector *vals;
  struct stack *padding;
  struct stack *shadow;
} stack;

typedef struct asm_fn {
  char *name;
  vector *code;
  stack *stk;
} asm_fn;

typedef struct {
  char *name;
  vector *fields;
  vector *types;
} asm_struct;

typedef struct asm_prog {
  vector *data;
  vector *fns;
  vector *const_names;
  vector *const_vals;
  int jmp_ctr;
  ctx *ctx;
  stack *stk;
} asm_prog;

asm_prog *gen_asm_ir(vector *cmds, ctx *global);
void cmd_asmgen(asm_prog *prog, asm_fn *fn, cmd *c);

#endif
