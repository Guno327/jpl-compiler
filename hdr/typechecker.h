#include "ctx.h"
#include "t.h"
#include "vector.h"
#ifndef TYPECHECKER_H
#define TYPECHECKER_H

void typecheck(vector *program);
void typeof_expr(expr *e, ctx *c);

#endif
