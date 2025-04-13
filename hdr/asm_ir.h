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

extern int opt;

typedef struct {
  vector *lvals;
  vector *args;
  vector *types;
  char *ret;
  t *ret_t;
  long ret_pos;
  long stk_size;
} call_conv;

typedef struct stack {
  struct asm_fn *fn;
  long size;
  vector *shadow;

  vector *names;
  vector *positions;
} stack;

typedef struct padding {
  long size;
} padding;

typedef struct asm_fn {
  char *name;
  vector *code;
  stack *stk;
  call_conv *call;
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
  long jmp_ctr;
  ctx *ctx;
  stack *stk;
} asm_prog;

asm_prog *gen_asm_ir(vector *cmds, ctx *global);
char *jmp_asmgen(asm_prog *prog);
void cmd_asmgen(asm_prog *prog, asm_fn *fn, cmd *c);
void expr_asmgen(asm_prog *prog, asm_fn *fn, expr *e);
void stmt_asmgen(asm_prog *prog, asm_fn *fn, stmt *s);

void stack_push(asm_fn *fn, char *reg);
t *stack_pop(asm_fn *fn, char *reg);
void stack_align(asm_fn *fn, long amount);
void stack_unalign(asm_fn *fn);
void stack_rechar(asm_fn *fn, t *type, long size);
void stack_free(asm_fn *fn, size_t bytes);

char *genconst(asm_prog *prog, char *val);
void assert_asmgen(asm_prog *prog, asm_fn *fn, char *cond, char *msg);
char *asm_prog_to_str(asm_prog *prog);

void stack_update_pos(asm_fn *fn, char *name, long pos);
void push_lval(asm_fn *fn, lval *lval, long base);
void let_asmgen(asm_prog *prog, asm_fn *fn, void *let, bool is_stmt);
void stack_alloc(asm_fn *fn, t *type);
void stack_copy(asm_fn *fn, t *type, char *start, char *end);
long stack_lookup(stack *stk, char *var);

bool is_int_reg(char *reg);
bool is_float_reg(char *reg);
bool is_bool_cast(if_expr *ife);
bool is_pow_2(long n);
bool is_opt_mult(binop_expr *bop);

long log2(long n);

#endif
