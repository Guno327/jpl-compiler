#include "ast.h"
#include "ctx.h"
#include "t.h"
#include "vector.h"
#ifndef TYPECHECK_H
#define TYPECHECK_H

void typecheck(vector *program);
void typeof_expr(expr *e, ctx *c);

#endif
