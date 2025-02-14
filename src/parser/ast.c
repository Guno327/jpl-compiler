#include "ast.h"
#include "alloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *cmd_to_str(cmd *cmd) {
  char *result = alloc(BUFSIZ);
  switch (cmd->type) {
  case READCMD:;
    read_cmd *rc = (read_cmd *)cmd->node;
    char *rc_lval = lval_to_str(rc->lval);
    sprintf(result, "(read_cmd %s %s)", rc->str, rc_lval);
    free(rc_lval);
    break;
  case WRITECMD:;
    write_cmd *wc = (write_cmd *)cmd->node;
    char *wc_expr = expr_to_str(wc->expr);
    sprintf(result, "(write_cmd %s %s)", wc_expr, wc->str);
    free(wc_expr);
    break;
  case LETCMD:;
    Letcmd *lc = (Letcmd *)cmd->node;
    char *lc_lval = lval_to_str(lc->lval);
    char *lc_expr = expr_to_str(lc->expr);
    sprintf(result, "(Letcmd %s %s)", lc_lval, lc_expr);
    free(lc_lval);
    free(lc_expr);
    break;
  case ASSERTCMD:;
    assert_cmd *ac = (assert_cmd *)cmd->node;
    char *ac_expr = expr_to_str(ac->expr);
    sprintf(result, "(assert_cmd %s %s)", ac_expr, ac->str);
    break;
  case PRINTCMD:;
    print_cmd *pc = (print_cmd *)cmd->node;
    sprintf(result, "(print_cmd %s)", pc->str);
    break;
  case SHOWCMD:;
    show_cmd *sc = (show_cmd *)cmd->node;
    char *sc_expr = expr_to_str(sc->expr);
    sprintf(result, "(show_cmd %s)", sc_expr);
    free(sc_expr);
    break;
  case TIMECMD:;
    time_cmd *tc = (time_cmd *)cmd->node;
    char *tc_cmd = cmd_to_str(tc->cmd);
    sprintf(result, "(time_cmd %s)", tc_cmd);
    free(tc_cmd);
    break;
  case FNCMD:;
    fn_cmd *fc = (fn_cmd *)cmd->node;

    sprintf(result, "(fn_cmd %s ((", fc->var);
    if (fc->binds != NULL) {
      for (int i = 0; i < fc->binds_size; i++) {
        char *cur_lv = lval_to_str(fc->binds[i]->lval);
        char *cur_t = type_to_str(fc->binds[i]->type);
        strcat(result, cur_lv);
        strcat(result, " ");
        strcat(result, cur_t);
        if (i != fc->binds_size - 1)
          strcat(result, " ");
        free(cur_lv);
        free(cur_t);
      }
    }
    strcat(result, ")) ");

    char *fc_type = type_to_str(fc->type);
    strcat(result, fc_type);
    strcat(result, " ");
    free(fc_type);

    if (fc->stmts != NULL) {
      char *fc_stmts =
          list_to_str((void **)fc->stmts, fc->stmts_size, STMTLIST);
      strcat(result, fc_stmts);
      free(fc_stmts);
    }

    strcat(result, ")");
    break;
  case STRUCTCMD:;
    struct_cmd *stc = (struct_cmd *)cmd->node;
    sprintf(result, "(struct_cmd %s", stc->var);

    if (stc->vars != NULL && stc->types != NULL) {
      for (int i = 0; i < stc->vars_size; i++) {
        char *cur_v = stc->vars[i];
        char *cur_t = type_to_str(stc->types[i]);
        strcat(result, " ");
        strcat(result, cur_v);
        strcat(result, " ");
        strcat(result, cur_t);
        free(cur_v);
        free(cur_t);
      }
    }
    strcat(result, ")");
    break;
  }
  return result;
}

char *expr_to_str(expr *expr) {
  char *result = alloc(BUFSIZ);
  switch (expr->type) {
  case INTEXPR:;
    int_expr *ie = (int_expr *)expr->node;
    sprintf(result, "(int_expr %lu)", ie->val);
    break;
  case FLOATEXPR:;
    float_expr *fe = (float_expr *)expr->node;
    sprintf(result, "(float_expr %lu)", (long)fe->val);
    break;
  case TRUEEXPR:
    sprintf(result, "(true_expr)");
    break;
  case FALSEEXPR:
    sprintf(result, "(false_expr)");
    break;
  case VAREXPR:;
    var_expr *ve = (var_expr *)expr->node;
    sprintf(result, "(var_expr %s)", ve->var);
    break;
  case ARRAYLITERALEXPR:;
    array_literal_expr *ale = (array_literal_expr *)expr->node;
    if (ale->list != NULL) {
      char *ale_list = list_to_str((void **)ale->list->exprs,
                                   ale->list->exprs_size, EXPRLIST);
      sprintf(result, "(array_literal_expr %s)", ale_list);
      free(ale_list);
    } else {
      sprintf(result, "(array_literal_expr)");
    }
    break;
  case VOIDEXPR:
    sprintf(result, "(void_expr)");
    break;
  case STRUCTLITERALEXPR:;
    struct_literal_expr *sle = (struct_literal_expr *)expr->node;
    if (sle->list != NULL) {
      char *sle_list = list_to_str((void **)sle->list->exprs,
                                   sle->list->exprs_size, EXPRLIST);
      sprintf(result, "(struct_literal_expr %s %s)", sle->var, sle_list);
      free(sle_list);
    } else {
      sprintf(result, "(struct_literal_expr %s)", sle->var);
    }
    break;
  case DOTEXPR:;
    dot_expr *de = (dot_expr *)expr->node;
    char *de_expr = expr_to_str(de->expr);
    sprintf(result, "(dot_expr %s %s)", de_expr, de->var);
    free(de_expr);
    break;
  case ARRAYINDEXEXPR:;
    array_index_expr *aie = (array_index_expr *)expr->node;
    char *aie_expr = expr_to_str(aie->expr);

    if (aie->list != NULL) {
      char *aie_list = list_to_str((void **)aie->list->exprs,
                                   aie->list->exprs_size, EXPRLIST);
      sprintf(result, "(array_index_expr %s %s)", aie_expr, aie_list);
      free(aie_list);
    } else {
      sprintf(result, "(array_index_expr %s)", aie_expr);
    }
    free(aie_expr);
    break;
  case CALLEXPR:;
    call_expr *ce = (call_expr *)expr->node;
    if (ce->list != NULL) {
      char *ce_list =
          list_to_str((void **)ce->list->exprs, ce->list->exprs_size, EXPRLIST);
      sprintf(result, "(call_expr %s %s)", ce->var, ce_list);
      free(ce_list);
    } else {
      sprintf(result, "(call_expr %s)", ce->var);
    }
    break;
  case EXPR:;
    expr = expr->node;
    result = expr_to_str(expr);
    break;
  case UNOPEXPR:;
    unop_expr *ue = (unop_expr *)expr->node;
    char *u_op = uop_to_str(ue->op);
    char *u_rhs = expr_to_str(ue->rhs);
    sprintf(result, "(unop_expr %s %s)", u_op, u_rhs);
    free(u_rhs);
    break;
  case BINOPEXPR:;
    binop_expr *be = (binop_expr *)expr->node;
    char *b_op = bop_to_str(be->op);
    char *b_lhs = expr_to_str(be->lhs);
    char *b_rhs = expr_to_str(be->rhs);
    sprintf(result, "(binop_expr %s %s %s)", b_lhs, b_op, b_rhs);
    free(b_lhs);
    free(b_rhs);
    break;
  case IFEXPR:;
    if_expr *ife = (if_expr *)expr->node;
    char *if_e = expr_to_str(ife->if_expr);
    char *then_e = expr_to_str(ife->then_expr);
    char *else_e = expr_to_str(ife->else_expr);
    sprintf(result, "(if_expr %s %s %s)", if_e, then_e, else_e);
    free(if_e);
    free(else_e);
    free(then_e);
    break;
  case ARRAYLOOPEXPR:;
    array_loop_expr *aloop = (array_loop_expr *)expr->node;
    sprintf(result, "(array_loop_expr");
    for (int i = 0; i < aloop->vars_size; i++) {
      strcat(result, " ");
      strcat(result, aloop->vars[i]);
      strcat(result, " ");

      char *cur_e = expr_to_str(aloop->list->exprs[i]);
      strcat(result, cur_e);
      free(cur_e);
    }
    char *a_final_e = expr_to_str(aloop->expr);
    strcat(result, " ");
    strcat(result, a_final_e);
    strcat(result, ")");
    free(a_final_e);
    break;
  case SUMLOOPEXPR:;
    sum_loop_expr *sloop = (sum_loop_expr *)expr->node;
    sprintf(result, "(sum_loop_expr");
    for (int i = 0; i < sloop->vars_size; i++) {
      strcat(result, " ");
      strcat(result, sloop->vars[i]);
      strcat(result, " ");

      char *cur_e = expr_to_str(sloop->list->exprs[i]);
      strcat(result, cur_e);
      free(cur_e);
    }
    char *s_final_e = expr_to_str(sloop->expr);
    strcat(result, " ");
    strcat(result, s_final_e);
    strcat(result, ")");
    free(s_final_e);
    break;
  }
  return result;
}

char *uop_to_str(u_op op) {
  switch (op) {
  case NOTOP:
    return "!";
  case NEGOP:
    return "-";
  }
  return NULL;
}

char *bop_to_str(b_op op) {
  switch (op) {
  case MULTOP:
    return "*";
  case DIVOP:
    return "/";
  case MODOP:
    return "%";
  case ADDOP:
    return "+";
  case SUBOP:
    return "-";
  case GTOP:
    return ">";
  case LTOP:
    return "<";
  case GEOP:
    return ">=";
  case LEOP:
    return "<=";
  case EQOP:
    return "==";
  case NEOP:
    return "!=";
  case ANDOP:
    return "&&";
  case OROP:
    return "||";
  }
  return NULL;
}

char *lval_to_str(lval *lval) {
  char *result = alloc(BUFSIZ);
  switch (lval->type) {
  case VARLVALUE:;
    var_lval *vlv = (var_lval *)lval->node;
    sprintf(result, "(var_lval %s)", vlv->var);
    break;
  case ARRAYLVALUE:;
    array_lval *alv = (array_lval *)lval->node;
    if (alv->vars != NULL) {
      char *alv_vars = list_to_str((void **)alv->vars, alv->vars_size, VARLIST);
      sprintf(result, "(array_lval %s %s)", alv->var, alv_vars);
      free(alv_vars);
    } else {
      sprintf(result, "(array_lval %s)", alv->var);
    }
    break;
  }
  return result;
}

char *stmt_to_str(stmt *stmt) {
  char *result = alloc(BUFSIZ);
  switch (stmt->type) {
  case LETSTMT:;
    let_stmt *ls = (let_stmt *)stmt->node;
    char *ls_lval = lval_to_str(ls->lval);
    char *ls_expr = expr_to_str(ls->expr);
    sprintf(result, "(let_stmt %s %s)", ls_lval, ls_expr);
    free(ls_lval);
    free(ls_expr);
    break;
  case ASSERTSTMT:;
    assert_stmt *as = (assert_stmt *)stmt->node;
    char *as_expr = expr_to_str(as->expr);
    sprintf(result, "(assert_stmt %s %s)", as_expr, as->str);
    free(as_expr);
    break;
  case RETURNSTMT:;
    return_stmt *rs = (return_stmt *)stmt->node;
    char *rs_expr = expr_to_str(rs->expr);
    sprintf(result, "(return_stmt %s)", rs_expr);
    free(rs_expr);
    break;
  }
  return result;
}

char *type_to_str(type *type) {
  char *result = alloc(BUFSIZ);
  switch (type->type) {
  case INTTYPE:
    sprintf(result, "(int_type)");
    break;
  case FLOATTYPE:
    sprintf(result, "(float_type)");
    break;
  case BOOLTYPE:
    sprintf(result, "(bool_type)");
    break;
  case ARRAYTYPE:;
    array_type *at = (array_type *)type->node;
    char *at_t = type_to_str(at->type);
    sprintf(result, "(array_type %s %d)", at_t, at->rank);
    free(at);
    break;
  case STRUCTTYPE:;
    struct_type *st = (struct_type *)type->node;
    sprintf(result, "(struct_type %s)", st->var);
    break;
  case VOIDTYPE:
    sprintf(result, "(void_type)");
    break;
  }
  return result;
}

char *list_to_str(void **list, size_t size, list_t list_t) {
  char *result = NULL;
  size_t len = 0;

  for (int i = 0; i < size; i++) {
    char *cur = NULL;
    switch (list_t) {
    case CMDLIST:;
      cmd **cmd_list = (cmd **)list;
      cur = cmd_to_str(cmd_list[i]);
      break;
    case EXPRLIST:;
      expr **expr_list = (expr **)list;
      cur = expr_to_str(expr_list[i]);
      break;
    case LVALUELIST:;
      lval **lval_list = (lval **)list;
      cur = lval_to_str(lval_list[i]);
      break;
    case STMTLIST:;
      stmt **stmt_list = (stmt **)list;
      cur = stmt_to_str(stmt_list[i]);
      break;
    case TYPELIST:;
      type **type_list = (type **)list;
      cur = type_to_str(type_list[i]);
      break;
    case VARLIST:;
      char **var_list = (char **)list;
      char *cur_var = var_list[i];
      cur = alloc(strlen(cur_var) + 1);
      memcpy(cur, cur_var, strlen(cur_var));
      break;
    default:
      return NULL;
    }

    char *tmp = NULL;
    while (tmp == NULL)
      tmp = realloc(result, len + strlen(cur) + 2);
    result = tmp;
    memset(result + len, 0, strlen(cur) + 2);
    len += strlen(cur) + 2;

    strcat(result, cur);
    free(cur);
    if (i != size - 1)
      strcat(result, " ");
  }

  return result;
}
