#include "c_ir.h"
#include "compiler_error.h"
#include "safe.h"
#include "typecheck.h"
#include "vector.h"
#include "vector_get.h"
#include <stdio.h>
#include <string.h>

void cmd_gencode(c_prog *prog, c_fn *fn, cmd *c) {
  switch (c->type) {
  case SHOWCMD:;
    show_cmd *sc = (show_cmd *)c->node;
    char *sc_sym = expr_gencode(prog, fn, sc->expr);

    char *sc_code = safe_alloc(1);
    sc_code = safe_strcat(sc_code, "show(\"");
    char *sc_type_str = genshowt(sc->expr->t_type);
    sc_code = safe_strcat(sc_code, sc_type_str);
    free(sc_type_str);
    sc_code = safe_strcat(sc_code, "\", &");
    sc_code = safe_strcat(sc_code, sc_sym);
    sc_code = safe_strcat(sc_code, ");\n");

    vector_append(fn->code, sc_code);
    break;
  case LETCMD:;
    let_cmd *lc = (let_cmd *)c->node;
    char *lc_sym = expr_gencode(prog, fn, lc->expr);
    char *lc_var = NULL;
    switch (lc->lval->type) {
    case VARLVALUE:
      lc_var = ((var_lval *)lc->lval->node)->var;
      break;
    case ARRAYLVALUE:
      lc_var = ((array_lval *)lc->lval->node)->var;
      break;
    }
    vector_append(fn->jpl_names, lc_var);
    vector_append(fn->c_names, lc_sym);

    if (lc->lval->type == ARRAYLVALUE) {
      array_lval *alv = (array_lval *)lc->lval->node;
      for (int i = 0; i < alv->vars->size; i++) {
        vector_append(fn->jpl_names, vector_get_str(alv->vars, i));
        char *c_name = safe_alloc(BUFSIZ);
        sprintf(c_name, "%s.d%d", lc_sym, i);
        vector_append(fn->c_names, c_name);
      }
    }
    break;
  case PRINTCMD:;
    print_cmd *pc = (print_cmd *)c->node;
    char *pc_code = safe_alloc(1);
    pc_code = safe_strcat(pc_code, "print(");
    pc_code = safe_strcat(pc_code, pc->str);
    pc_code = safe_strcat(pc_code, ");\n");

    vector_append(fn->code, pc_code);
    break;
  case ASSERTCMD:;
    assert_cmd *ac = (assert_cmd *)c->node;
    char *ac_sym = expr_gencode(prog, fn, ac->expr);

    char *ac_code = safe_alloc(1);
    ac_code = safe_strcat(ac_code, "if (0 != ");
    ac_code = safe_strcat(ac_code, ac_sym);
    free(ac_sym);
    ac_code = safe_strcat(ac_code, ")\n");

    char *pass_jmp = genjmp(prog);
    ac_code = safe_strcat(ac_code, "goto ");
    ac_code = safe_strcat(ac_code, pass_jmp);
    ac_code = safe_strcat(ac_code, ";\n");

    ac_code = safe_strcat(ac_code, "fail_assertion(");
    ac_code = safe_strcat(ac_code, ac->str);
    ac_code = safe_strcat(ac_code, ");\n");
    ac_code = safe_strcat(ac_code, pass_jmp);
    free(pass_jmp);
    ac_code = safe_strcat(ac_code, ":;\n");

    vector_append(fn->code, ac_code);
    break;
  case STRUCTCMD:;
    struct_cmd *stc = (struct_cmd *)c->node;
    c_struct *s = safe_alloc(sizeof(c_struct));
    s->name = stc->var;
    s->types = safe_alloc(sizeof(vector));
    s->fields = safe_alloc(sizeof(vector));
    vector_init(s->types, stc->types->size, STRVECTOR);
    vector_init(s->fields, stc->vars->size, STRVECTOR);

    for (int i = 0; i < stc->vars->size; i++) {
      char *cur_field = vector_get_str(stc->vars, i);
      type *cur_type = vector_get_type(stc->types, i);
      char *cur_type_str = gent(prog, fn, typeof_type(cur_type, prog->ctx));

      vector_append(s->fields, cur_field);
      vector_append(s->types, cur_type_str);
    }

    vector_append(prog->structs, s);
    break;
  case WRITECMD:;
    write_cmd *wc = (write_cmd *)c->node;
    char *wc_sym = expr_gencode(prog, fn, wc->expr);
    char *wc_code = safe_alloc(1);
    wc_code = safe_strcat(wc_code, "write_image(");
    wc_code = safe_strcat(wc_code, wc_sym);
    wc_code = safe_strcat(wc_code, ", ");
    wc_code = safe_strcat(wc_code, wc->str);
    wc_code = safe_strcat(wc_code, ");\n");
    vector_append(fn->code, wc_code);
    break;
  case READCMD:;
    read_cmd *rc = (read_cmd *)c->node;
    char *rc_sym = gensym(fn);
    char *rc_code = safe_alloc(1);

    rc_code = safe_strcat(rc_code, "_a2_rgba ");
    rc_code = safe_strcat(rc_code, rc_sym);
    rc_code = safe_strcat(rc_code, " = read_image(");
    rc_code = safe_strcat(rc_code, rc->str);
    rc_code = safe_strcat(rc_code, ");\n");

    vector_append(fn->c_names, rc_sym);
    switch (rc->lval->type) {
    case VARLVALUE:;
      var_lval *vlv = (var_lval *)rc->lval->node;
      vector_append(fn->jpl_names, vlv->var);
      break;
    case ARRAYLVALUE:;
      array_lval *alv = (array_lval *)rc->lval->node;
      vector_append(fn->jpl_names, alv->var);

      for (int i = 0; i < alv->vars->size; i++) {
        char *cur_var = vector_get_str(alv->vars, i);
        char *c_name = safe_alloc(BUFSIZ);
        sprintf(c_name, "%s.d%d", rc_sym, i);

        rc_code = safe_strcat(rc_code, "int64_t ");
        rc_code = safe_strcat(rc_code, cur_var);
        rc_code = safe_strcat(rc_code, " = ");
        rc_code = safe_strcat(rc_code, c_name);
        rc_code = safe_strcat(rc_code, ";\n");

        vector_append(fn->jpl_names, cur_var);
        vector_append(fn->c_names, c_name);
      }
      break;
    }

    vector_append(fn->code, rc_code);
    break;
  case TIMECMD:;
    time_cmd *tc = (time_cmd *)c->node;
    char *start_sym = gensym(fn);
    char *tc_code = safe_alloc(1);

    tc_code = safe_strcat(tc_code, "double ");
    tc_code = safe_strcat(tc_code, start_sym);
    tc_code = safe_strcat(tc_code, " = get_time();\n");
    vector_append(fn->code, tc_code);
    tc_code = safe_alloc(1);

    cmd_gencode(prog, fn, tc->cmd);

    char *end_sym = gensym(fn);
    tc_code = safe_strcat(tc_code, "double ");
    tc_code = safe_strcat(tc_code, end_sym);
    tc_code = safe_strcat(tc_code, " = get_time();\n");

    tc_code = safe_strcat(tc_code, "print_time(");
    tc_code = safe_strcat(tc_code, end_sym);
    free(end_sym);
    tc_code = safe_strcat(tc_code, " - ");
    tc_code = safe_strcat(tc_code, start_sym);
    free(start_sym);
    tc_code = safe_strcat(tc_code, ");\n");

    vector_append(fn->code, tc_code);
    break;
  case FNCMD:;
    fn_cmd *fc = (fn_cmd *)c->node;
    c_fn *fn_def = safe_alloc(sizeof(c_fn));

    fn_def->name = safe_alloc(strlen(fc->var) + 1);
    memcpy(fn_def->name, fc->var, strlen(fc->var));

    fn_def->name_ctr = 0;
    fn_def->c_names = safe_alloc(sizeof(vector));
    fn_def->jpl_names = safe_alloc(sizeof(vector));
    vector_init(fn_def->c_names, 8, STRVECTOR);
    vector_init(fn_def->jpl_names, 8, STRVECTOR);

    // Figure out return type
    char *fc_ret = gent(prog, fn, type_to_t(fc->type));
    fn_def->ret_type = fc_ret;

    // Create args list
    char *args_list = safe_alloc(1);
    for (int i = 0; i < fc->binds->size; i++) {
      binding *cur_bind = vector_get_binding(fc->binds, i);
      char *cur_type = gent(prog, fn, type_to_t(cur_bind->type));

      char *cur_var = NULL;
      switch (cur_bind->lval->type) {
      case VARLVALUE:;
        var_lval *vlv = (var_lval *)cur_bind->lval->node;
        cur_var = vlv->var;
        break;
      case ARRAYLVALUE:;
        array_lval *alv = (array_lval *)cur_bind->lval->node;
        cur_var = alv->var;

        if (alv->vars->size != 0) {
          for (int i = 0; i < alv->vars->size; i++) {
            char *cur_jpl = vector_get_str(alv->vars, i);
            char *cur_c = safe_alloc(BUFSIZ);
            sprintf(cur_c, "%s.d%d", alv->var, i);

            vector_append(fn_def->jpl_names, cur_jpl);
            vector_append(fn_def->c_names, cur_c);
          }
        }
        break;
      }

      args_list = safe_strcat(args_list, cur_type);
      args_list = safe_strcat(args_list, " ");
      args_list = safe_strcat(args_list, cur_var);

      if (i != fc->binds->size - 1)
        args_list = safe_strcat(args_list, ", ");

      // Binds args for stmt calls
      vector_append(fn_def->c_names, cur_var);
      vector_append(fn_def->jpl_names, cur_var);
    }
    fn_def->args_list = args_list;

    // Because the autograder is silly we have to use jpl_names to refer to
    // global vars :(
    fn_def->parent = NULL;

    // Create code from stmts
    bool found_ret = false;
    fn_def->code = safe_alloc(sizeof(vector));
    long stmts = fc->stmts->size == 0 ? 1 : fc->stmts->size;
    vector_init(fn_def->code, stmts, STRVECTOR);
    for (int i = 0; i < fc->stmts->size; i++) {
      stmt *cur_stmt = vector_get_stmt(fc->stmts, i);
      bool is_ret = stmt_gencode(prog, fn_def, cur_stmt);
      if (!found_ret && is_ret)
        found_ret = true;
    }
    // Implicit return (is void thanks to typecheck)
    if (!found_ret) {
      char *void_ret = safe_alloc(7);
      memcpy(void_ret, "void_t", 6);
      char *ret_sym = gensym(fn_def);
      char *ret_code = safe_alloc(1);
      ret_code = safe_strcat(ret_code, "void_t ");
      ret_code = safe_strcat(ret_code, ret_sym);
      ret_code = safe_strcat(ret_code, " = {};\n");
      ret_code = safe_strcat(ret_code, "return ");
      ret_code = safe_strcat(ret_code, ret_sym);
      ret_code = safe_strcat(ret_code, ";\n");
      vector_append(fn_def->code, ret_code);
    }

    vector_append(prog->fns, fn_def);
    break;
  }
}
