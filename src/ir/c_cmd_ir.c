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
    char *end_sym = gensym(fn);
    char *tc_code = safe_alloc(1);

    tc_code = safe_strcat(tc_code, "double ");
    tc_code = safe_strcat(tc_code, start_sym);
    tc_code = safe_strcat(tc_code, " = get_time();\n");
    vector_append(fn->code, tc_code);
    tc_code = safe_alloc(1);

    cmd_gencode(prog, fn, tc->cmd);

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
  default:;
    char *msg = safe_alloc(BUFSIZ);
    sprintf(msg, "cmd not yet implemented");
    ir_error(msg);
  }
}
