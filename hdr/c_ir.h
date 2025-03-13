#include "ast.h"
#include "ctx.h"
#include "t.h"
#include "vector.h"
#ifndef C_IR_H
#define C_IR_H

struct c_prog;
struct c_fn;
#define C_HDR                                                                  \
  "#include <math.h>\n#include <stdbool.h>\n#include <stdint.h>\n#include "    \
  "<stdio.h>\n#include \"rt/runtime.h\"\n\ntypedef struct { } void_t;\n\n"

typedef struct c_fn {
  char *name;
  char *ret_type;
  char *args_list;
  vector *code;
  struct c_fn *parent;
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
char *genjmp(c_prog *main);
c_prog *gen_c_ir(vector *cmds, ctx *global);
char *c_prog_to_str(c_prog *prog);
char *expr_gencode(c_prog *prog, c_fn *main, expr *e);
void cmd_gencode(c_prog *prog, c_fn *main, cmd *c);
void stmt_gencode(c_prog *prog, c_fn *main, stmt *s);
char *genarray(c_prog *prog, c_fn *fn, t *type, int size);
char *gent(c_prog *prog, c_fn *fn, t *t);
char *genshowt(t *t);
char *jpl_to_c(c_fn *fn, char *jpl_name);

#endif
