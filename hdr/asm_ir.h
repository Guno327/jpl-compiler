#include "ast.h"
#include "ctx.h"
#include "t.h"
#include "vector.h"
#ifndef ASM_IR_H
#define ASM_IR_H

#define ASM_GLOBALS "global jpl_main\nglobal _jpl_main"
#define ASM_EXTERNS                                                            \
  "extern _fail_assertion\nextern _jpl_alloc\nextern _get_time\nextern "       \
  "_show\nextern _print\nextern _print_time\nextern _read_image\nextern "      \
  "_write_image\nextern _fmod\nextern _sqrt\nextern _exp\nextern "             \
  "_sin\nextern _cos\nextern _tan\nextern _asin\nextern _acos\nextern "        \
  "_atan\nextern _log\nextern _pow\nextern _atan2\nextern _to_int\nextern "    \
  "_to_float"

struct asm_prog;
struct asm_fn;
struct stack;

typedef struct stack {
  struct asm_fn *fn;
  size_t size;
  vector *shadow;
} stack;

typedef struct padding {
  size_t size;
} padding;

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
void expr_asmgen(asm_prog *prog, asm_fn *fn, expr *e);

void stack_push(asm_fn *fn, char *reg, t *type);
t *stack_pop(asm_fn *fn, char *reg);
void stack_align(asm_fn *fn, size_t amount);
void stack_unalign(asm_fn *fn, size_t amount);

char *genconst(asm_prog *prog, char *val);
void assert_asmgen(asm_prog *prog, asm_fn *fn, char *msg);
char *asm_prog_to_str(asm_prog *prog);

#endif
