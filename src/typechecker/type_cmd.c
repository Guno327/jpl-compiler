#include "compiler_error.h"
#include "ctx.h"
#include "safe.h"
#include "stdio.h"
#include "typecheck.h"
#include "vector.h"
#include "vector_get.h"
#include <string.h>

void type_cmd(cmd *c, ctx *global) {
  // get rgba info for read/write
  info *rgba = check_ctx(global, "rgba");

  struct_info *rgba_info = (struct_info *)rgba->node;
  switch (c->type) {
  case STRUCTCMD:;
    struct_cmd *sc = (struct_cmd *)c->node;

    // Check if already defined
    info *exists = check_ctx(global, sc->var);
    if (exists != NULL) {
      char *msg = safe_alloc(BUFSIZ);
      sprintf(msg, "Symbol '%s' already defined", sc->var);
      typecheck_error(msg, sc->start);
    }

    // Alloc info, check for duplicates
    struct_info *s_info = safe_alloc(sizeof(struct_info));
    s_info->name = sc->var;
    s_info->vars = sc->vars;
    for (long i = 0; i < s_info->vars->size; i++) {
      char *cur_var = vector_get_str(s_info->vars, i);
      for (long j = 0; j < s_info->vars->size; j++) {
        if (j != i && !strcmp(cur_var, vector_get_str(s_info->vars, j))) {
          char *msg = safe_alloc(BUFSIZ);
          sprintf(msg, "Symbol '%s' already defined", cur_var);
          typecheck_error(msg, vector_get_type(sc->types, i)->start);
        }
      }
    }

    // Get struct fields
    vector *s_ts = safe_alloc(sizeof(vector));
    vector_init(s_ts, 8, TVECTOR);
    for (long i = 0; i < sc->types->size; i++) {
      t *cur = typeof_type(vector_get_type(sc->types, i), global);
      vector_append(s_ts, cur);
    }

    // Add struct to scope
    s_info->ts = s_ts;
    vector_append(global->structs, s_info);
    break;
  case SHOWCMD:;
    show_cmd *shc = (show_cmd *)c->node;
    t *shc_expr = typeof_expr(shc->expr, global);
    shc->expr->t_type = shc_expr;
    break;
  case READCMD:;
    read_cmd *rc = (read_cmd *)c->node;

    // Create type for var
    t *rc_t = safe_alloc(sizeof(t));
    rc_t->type = ARRAY_T;
    rc_t->info = safe_alloc(sizeof(array_info));
    array_info *rc_array = (array_info *)rc_t->info;
    rc_array->rank = 2;
    rc_array->type = safe_alloc(sizeof(t));
    rc_array->type->type = STRUCT_T;
    rc_array->type->info = rgba_info;

    // Add to lval
    type_lval(rc->lval, rc_t, global);
    break;
  case WRITECMD:;
    write_cmd *wc = (write_cmd *)c->node;
    t *wc_expr = typeof_expr(wc->expr, global);
    if (strcmp(t_to_str(wc_expr), "(ArrayType (StructType rgba) 2)")) {
      char *msg = safe_alloc(BUFSIZ);
      sprintf(msg, "Expected type (StructType rgba) got %s", t_to_str(wc_expr));
      typecheck_error(msg, wc->expr->start);
    }
    wc->expr->t_type = wc_expr;
    break;
  case LETCMD:;
    let_cmd *lc = (let_cmd *)c->node;
    t *lc_t = typeof_expr(lc->expr, global);
    lc->expr->t_type = lc_t;
    type_lval(lc->lval, lc_t, global);
    break;
  case ASSERTCMD:;
    assert_cmd *ac = (assert_cmd *)c->node;
    t *ac_expr = typeof_expr(ac->expr, global);
    if (ac_expr->type != BOOL_T) {
      char *msg = safe_alloc(BUFSIZ);
      sprintf(msg, "Expected type (BoolType) got %s", t_to_str(ac_expr));
      typecheck_error(msg, ac->expr->start);
    }
    ac->expr->t_type = ac_expr;
    break;
  case PRINTCMD:
    // Nothing to typecheck
    break;
  case TIMECMD:;
    time_cmd *tc = (time_cmd *)c->node;
    type_cmd(tc->cmd, global);
    break;
  case FNCMD:;
    fn_cmd *fc = (fn_cmd *)c->node;

    // Check if already defined
    info *fn_exists = check_ctx(global, fc->var);
    if (fn_exists != NULL) {
      char *msg = safe_alloc(BUFSIZ);
      sprintf(msg, "Symbol '%s' already defined", fc->var);
      typecheck_error(msg, fc->start);
    }

    // Get fn name
    fn_info *fc_info = safe_alloc(sizeof(fn_info));
    fc_info->name = fc->var;

    // Get return type
    t *fc_ret = typeof_type(fc->type, global);
    fc_info->ret = fc_ret;

    // Get types for args and add to inner scope
    fc_info->args = fc->binds;
    fc_info->ctx = setup_ctx();
    fc_info->ctx->parent = global;
    for (long i = 0; i < fc->binds->size; i++) {
      binding *b = vector_get_binding(fc->binds, i);
      t *bt = typeof_type(b->type, global);
      type_lval(b->lval, bt, fc_info->ctx);
    }

    // Add fn to scope BEFORE stmt eval to allow for recursion
    vector_append(global->fns, fc_info);

    // Check statments for return, use inner scope
    bool found_return = false;
    for (long i = 0; i < fc->stmts->size; i++) {
      stmt *s = vector_get_stmt(fc->stmts, i);
      t *st = typeof_stmt(s, fc_info->ctx);
      if (s->type == RETURNSTMT) {
        if (strcmp(t_to_str(st), t_to_str(fc_ret))) {
          char *msg = safe_alloc(BUFSIZ);
          sprintf(msg, "Function is supposed to return %s, returns %s",
                  t_to_str(fc_ret), t_to_str(st));
          typecheck_error(msg, s->start);
        }
        found_return = true;
      }
    }

    if (!found_return && fc_info->ret->type != VOID_T) {
      char *msg = safe_alloc(BUFSIZ);
      sprintf(msg, "Reached end of non-void function without a return");
      typecheck_error(msg, fc->start);
    }
    break;
  }
}
