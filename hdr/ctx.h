#include "t.h"
#ifndef CTX_H
#define CTX_H

struct ctx;

typedef struct ctx {
  struct ctx *parent;
  // Only structs rn, HW6 dnt have `let`
  vector *structs;
} ctx;

#endif
