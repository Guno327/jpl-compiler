#include "compiler_error.h"
#include "ctx.h"
#include "safe.h"
#include "typecheck.h"
#include "vector.h"
#include "vector_get.h"
#include <stdio.h>

t *typeof_stmt(stmt *s, ctx *c) {
  t *result = NULL;
  switch (s->type) {
  case LETSTMT:;
    let_stmt *ls = (let_stmt *)s->node;
    t *ls_t = typeof_expr(ls->expr, c);
    ls->expr->t_type = ls_t;
    type_lval(ls->lval, ls_t, c);
    result = ls_t;
    break;
  case ASSERTSTMT:;
    assert_stmt *as = (assert_stmt *)s->node;
    t *as_t = typeof_expr(as->expr, c);
    if (as_t->type != BOOL_T) {
      char *msg = safe_alloc(BUFSIZ);
      sprintf(msg, "Expected type (BoolType) got %s", t_to_str(as_t));
      typecheck_error(msg, as->expr->start);
    }
    as->expr->t_type = as_t;
    result = as_t;
    break;
  case RETURNSTMT:;
    return_stmt *rs = (return_stmt *)s->node;
    t *rs_t = typeof_expr(rs->expr, c);
    rs->expr->t_type = rs_t;
    result = rs_t;
    break;
  }
  return result;
}
