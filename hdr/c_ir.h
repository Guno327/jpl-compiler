#include "ast.h"
#include "ctx.h"
#include "t.h"
#include "vector.h"
#ifndef C_IR_H
#define C_IR_H

struct c_prog;

typedef struct {
  char *name;
  vector *code;
  struct c_prog *parent;
  int name_ctr;
  vector *jpl_names;
  vector *c_names;
} c_fn;

typedef struct {
  char *name;
  vector *fields;
  vector *types;
} c_struct;

typedef struct c_prog {
  vector *fns;
  vector *structs;
  int jump_ctr;
  ctx *ctx;
} c_prog;

char *gensym(c_fn *fn);
c_prog *gen_c_ir(vector *cmds, ctx *global);
char *c_prog_to_str(c_prog *prog);
char *expr_gencode(c_prog *prog, c_fn *main, expr *e);
char *genarray(c_prog *prog, c_fn *main, t *type, int size);
char *gentype(t *t);
char *genstruct(c_prog *prog, c_fn *main, char *jpl_name, vector *names);

#endif
