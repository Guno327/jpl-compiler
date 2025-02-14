#include "ast.h"
#include "ctx.h"
#include "t.h"
#include "vector.h"
#ifndef TYPECHECK_H
#define TYPECHECK_H

void typecheck(vector *program);
t *typeof_type(type *t, ctx *c);
t *typeof_expr(expr *e, ctx *c);

#endif
