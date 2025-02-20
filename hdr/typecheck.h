#include "ast.h"
#include "ctx.h"
#include "t.h"
#include "vector.h"
#ifndef TYPECHECK_H
#define TYPECHECK_H

void typecheck(vector *program);
ctx *setup_global_ctx();
void type_cmd(cmd *c, ctx *global);
void type_lval(lval *lv, t *type, ctx *c);
t *typeof_stmt(stmt *s, ctx *c);
t *typeof_type(type *t, ctx *c);
t *typeof_expr(expr *e, ctx *c);

#endif
