#include "alloc.h"
#include "compiler_error.h"
#include "ctx.h"
#include "stdio.h"
#include "typecheck.h"
#include "vector.h"
#include "vector_get.h"

void type_cmd(cmd *c, ctx *global) {
  switch (c->type) {
  case STRUCTCMD:;
    struct_cmd *sc = (struct_cmd *)c->node;

    struct_info *s_info = alloc(sizeof(struct_info));
    s_info->name = sc->var;
    s_info->vars = sc->vars;

    vector *s_ts = alloc(sizeof(vector));
    vector_init(s_ts, 8, TVECTOR);
    for (int i = 0; i < sc->types->size; i++) {
      t *cur = typeof_type(vector_get_type(sc->types, i), global);
      vector_append(s_ts, cur);
    }
    s_info->ts = s_ts;
    vector_append(global->structs, s_info);
    break;
  case SHOWCMD:;
    show_cmd *shc = (show_cmd *)c->node;
    t *shc_expr = typeof_expr(shc->expr, global);
    shc->expr->t_type = shc_expr;
    break;
  case READCMD:;
    break;
  case WRITECMD:;
    write_cmd *wc = (write_cmd *)c->node;
    t *wc_expr = typeof_expr(wc->expr, global);
    wc->expr->t_type = wc_expr;
    break;
  case LETCMD:;
    let_cmd *lc = (let_cmd *)c->node;
    t *lc_t = typeof_expr(lc->expr, global);
    lc->expr->t_type = lc_t;
    lc->lval->t_type = lc_t;

    break;
  case ASSERTCMD:;
    assert_cmd *ac = (assert_cmd *)c->node;
    t *ac_expr = typeof_expr(ac->expr, global);
    if (ac_expr->type != BOOL_T) {
      char *msg = alloc(BUFSIZ);
      sprintf(msg, "Expected type (BoolType) got %s", t_to_str(ac_expr));
      typecheck_error(msg, ac->expr->start);
    }
    ac->expr->t_type = ac_expr;
    break;
  case PRINTCMD:
    break;
  case TIMECMD:;
    time_cmd *tc = (time_cmd *)c->node;
    type_cmd(tc->cmd, global);
    break;
  case FNCMD:;
    fn_cmd *fc = (fn_cmd *)c->node;
    fn_info *fc_info = alloc(sizeof(fn_info));
    fc_info->name = fc->var;

    t *fc_ret = typeof_type(fc->type, global);
    fc_info->ret = fc_ret;

    fc_info->args = alloc(sizeof(vector));
    vector_init(fc_info->args, 8, TVECTOR);
    for (int i = 0; i < fc->binds->size; i++) {
      binding *b = vector_get_binding(fc->binds, i);
      t *b_t = typeof_type(b->type, global);
      vector_append(fc_info->args, b_t);
    }

    for (int i = 0; i < fc->stmts->size; i++) {
      stmt *s = vector_get_stmt(fc->stmts, i);
      t *s_t = typeof_stmt(s, global);
      s->t_type = s_t;
    }
    break;
  }
}
