#include "t.h"
#ifndef CTX_H
#define CTX_H

struct ctx;

typedef struct ctx {
  struct ctx *parent;
  vector *structs;
  vector *arrays;
  vector *fns;
  vector *vars;
} ctx;

typedef enum { STRUCTINFO, ARRAYINFO, FNINFO, VARINFO } info_type;
typedef struct {
  info_type type;
  void *node;
} info;

ctx *setup_ctx();
info *check_ctx(ctx *c, char *name);
char *ctx_to_str(ctx *c);

#endif
