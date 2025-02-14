#include "t.h"
#ifndef CTX_H
#define CTX_H

struct ctx;

typedef struct ctx {
  struct ctx *parent;
  vector *structs;
  vector *arrays;
} ctx;

#endif
