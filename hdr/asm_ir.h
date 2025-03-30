#include "ast.h"
#include "ctx.h"
#include "t.h"
#include "vector.h"
#ifndef ASM_IR_H
#define ASM_IR_H

#define ASM_GLOBALS "global jpl_main\nglobal _jpl_main\n"
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

  vector *names;
  vector *positions;
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

#define int_registers [ "rdi", "rsi", "rdx", "rcx", "r8", "r9" ]
#define float_registers                                                        \
  [ "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8" ]
typedef struct {
} call_conv;

asm_prog *gen_asm_ir(vector *cmds, ctx *global);
void cmd_asmgen(asm_prog *prog, asm_fn *fn, cmd *c);
void expr_asmgen(asm_prog *prog, asm_fn *fn, expr *e);

void stack_push(asm_fn *fn, char *reg);
t *stack_pop(asm_fn *fn, char *reg);
void stack_align(asm_fn *fn, size_t amount);
void stack_unalign(asm_fn *fn);
void stack_rechar(asm_fn *fn, t *type, size_t size);

char *genconst(asm_prog *prog, char *val);
void assert_asmgen(asm_prog *prog, asm_fn *fn, char *msg);
char *asm_prog_to_str(asm_prog *prog);

void push_lval(asm_fn *fn, lval *lval, size_t base);
void let_asmgen(asm_prog *prog, asm_fn *fn, void *let, bool is_stmt);
void stack_alloc(asm_fn *fn, t *type);
void stack_copy(asm_fn *fn, t *type, char *start, char *end);
size_t stack_lookup(stack *stk, char *var);
#endif
