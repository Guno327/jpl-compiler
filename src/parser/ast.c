#include "ast.h"
#include "safe.h"
#include "vector.h"
#include "vector_get.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *cmd_to_str(cmd *cmd) {
  char *result = safe_alloc(BUFSIZ);
  switch (cmd->type) {
  case READCMD:;
    read_cmd *rc = (read_cmd *)cmd->node;
    char *rc_lval = lval_to_str(rc->lval);
    result = safe_realloc_str(result, BUFSIZ + 1 + strlen(result) +
                                          strlen(rc->str) + strlen(rc_lval));
    sprintf(result, "(ReadCmd %s %s)", rc->str, rc_lval);
    free(rc_lval);
    break;
  case WRITECMD:;
    write_cmd *wc = (write_cmd *)cmd->node;
    char *wc_expr = expr_to_str(wc->expr);
    result = safe_realloc_str(result, BUFSIZ + 1 + strlen(result) +
                                          strlen(wc_expr) + strlen(wc->str));
    sprintf(result, "(WriteCmd %s %s)", wc_expr, wc->str);
    free(wc_expr);
    break;
  case LETCMD:;
    let_cmd *lc = (let_cmd *)cmd->node;
    char *lc_lval = lval_to_str(lc->lval);
    char *lc_expr = expr_to_str(lc->expr);
    result = safe_realloc_str(result, BUFSIZ + 1 + strlen(result) +
                                          strlen(lc_lval) + strlen(lc_expr));
    sprintf(result, "(LetCmd %s %s)", lc_lval, lc_expr);
    free(lc_lval);
    free(lc_expr);
    break;
  case ASSERTCMD:;
    assert_cmd *ac = (assert_cmd *)cmd->node;
    char *ac_expr = expr_to_str(ac->expr);
    result = safe_realloc_str(result, BUFSIZ + 1 + strlen(result) +
                                          strlen(ac_expr) + strlen(ac->str));
    sprintf(result, "(AssertCmd %s %s)", ac_expr, ac->str);
    break;
  case PRINTCMD:;
    print_cmd *pc = (print_cmd *)cmd->node;
    result =
        safe_realloc_str(result, BUFSIZ + 1 + strlen(result) + strlen(pc->str));
    sprintf(result, "(PrintCmd %s)", pc->str);
    break;
  case SHOWCMD:;
    show_cmd *sc = (show_cmd *)cmd->node;
    char *sc_expr = expr_to_str(sc->expr);
    result =
        safe_realloc_str(result, BUFSIZ + 1 + strlen(result) + strlen(sc_expr));
    sprintf(result, "(ShowCmd %s)", sc_expr);
    free(sc_expr);
    break;
  case TIMECMD:;
    time_cmd *tc = (time_cmd *)cmd->node;
    char *tc_cmd = cmd_to_str(tc->cmd);
    result =
        safe_realloc_str(result, BUFSIZ + 1 + strlen(result) + strlen(tc_cmd));
    sprintf(result, "(TimeCmd %s)", tc_cmd);
    free(tc_cmd);
    break;
  case FNCMD:;
    fn_cmd *fc = (fn_cmd *)cmd->node;

    result =
        safe_realloc_str(result, BUFSIZ + 1 + strlen(result) + strlen(fc->var));
    sprintf(result, "(FnCmd %s ((", fc->var);
    if (fc->binds != NULL) {
      for (int i = 0; i < fc->binds->size; i++) {
        binding *cur_bind = vector_get_binding(fc->binds, i);
        char *cur_lv = lval_to_str(cur_bind->lval);
        char *cur_t = type_to_str(cur_bind->type);
        result = safe_strcat(result, cur_lv);
        result = safe_strcat(result, " ");
        result = safe_strcat(result, cur_t);
        if (i != fc->binds->size - 1)
          result = safe_strcat(result, " ");
        free(cur_lv);
        free(cur_t);
      }
    }
    result = safe_strcat(result, ")) ");

    char *fc_type = type_to_str(fc->type);
    result = safe_strcat(result, fc_type);
    result = safe_strcat(result, " ");
    free(fc_type);

    if (fc->stmts != NULL) {
      char *fc_stmts = vector_to_str(fc->stmts);
      result = safe_strcat(result, fc_stmts);
      free(fc_stmts);
    }

    result = safe_strcat(result, ")");
    break;
  case STRUCTCMD:;
    struct_cmd *stc = (struct_cmd *)cmd->node;
    result = safe_realloc_str(result,
                              BUFSIZ + 1 + strlen(result) + strlen(stc->var));
    sprintf(result, "(StructCmd %s", stc->var);

    if (stc->vars != NULL && stc->types != NULL) {
      for (int i = 0; i < stc->vars->size; i++) {
        char *cur_v = vector_get_str(stc->vars, i);
        char *cur_t = type_to_str(vector_get_type(stc->types, i));
        result = safe_strcat(result, " ");
        result = safe_strcat(result, cur_v);
        result = safe_strcat(result, " ");
        result = safe_strcat(result, cur_t);
        free(cur_v);
        free(cur_t);
      }
    }
    result = safe_strcat(result, ")");
    break;
  }
  return result;
}

char *expr_to_str(expr *expr) {
  // Get type
  char *type_str = t_to_str(expr->t_type);
  if (type_str != NULL) {
    char *tmp = type_str;
    type_str = safe_alloc(BUFSIZ + 1);
    sprintf(type_str, " %s", tmp);
    free(tmp);
  } else {
    type_str = safe_alloc(1);
  }

  // rest
  char *result = safe_alloc(BUFSIZ);
  switch (expr->type) {
  case INTEXPR:;
    int_expr *ie = (int_expr *)expr->node;
    result = safe_realloc_str(result, BUFSIZ + 1 + strlen(result) +
                                          strlen(type_str) + 24);
    sprintf(result, "(IntExpr%s %ld)", type_str, ie->val);
    break;
  case FLOATEXPR:;
    float_expr *fe = (float_expr *)expr->node;
    result = safe_realloc_str(result, BUFSIZ + 1 + strlen(result) +
                                          strlen(type_str) + 24);
    sprintf(result, "(FloatExpr%s %ld)", type_str, (long)fe->val);
    break;
  case TRUEEXPR:
    result = safe_realloc_str(result,
                              BUFSIZ + 1 + strlen(result) + strlen(type_str));
    sprintf(result, "(TrueExpr%s)", type_str);
    break;
  case FALSEEXPR:
    result = safe_realloc_str(result,
                              BUFSIZ + 1 + strlen(result) + strlen(type_str));
    sprintf(result, "(FalseExpr%s)", type_str);
    break;
  case VAREXPR:;
    var_expr *ve = (var_expr *)expr->node;
    result = safe_realloc_str(result, BUFSIZ + 1 + strlen(result) +
                                          strlen(type_str) + strlen(ve->var));
    sprintf(result, "(VarExpr%s %s)", type_str, ve->var);
    break;
  case ARRAYLITERALEXPR:;
    array_literal_expr *ale = (array_literal_expr *)expr->node;
    if (ale->exprs->size != 0) {
      char *ale_list = vector_to_str(ale->exprs);
      result =
          safe_realloc_str(result, BUFSIZ + 1 + strlen(result) +
                                       strlen(type_str) + strlen(ale_list));
      sprintf(result, "(ArrayLiteralExpr%s %s)", type_str, ale_list);
      free(ale_list);
    } else {
      result = safe_realloc_str(result,
                                BUFSIZ + 1 + strlen(result) + strlen(type_str));
      sprintf(result, "(ArrayLiteralExpr%s)", type_str);
    }
    break;
  case VOIDEXPR:
    result = safe_realloc_str(result,
                              BUFSIZ + 1 + strlen(result) + strlen(type_str));
    sprintf(result, "(VoidExpr%s)", type_str);
    break;
  case STRUCTLITERALEXPR:;
    struct_literal_expr *sle = (struct_literal_expr *)expr->node;
    if (sle->exprs->size != 0) {
      char *sle_list = vector_to_str(sle->exprs);
      sprintf(result, "(StructLiteralExpr%s %s %s)", type_str, sle->var,
              sle_list);
      free(sle_list);
    } else {
      result =
          safe_realloc_str(result, BUFSIZ + 1 + strlen(result) +
                                       strlen(type_str) + strlen(sle->var));
      sprintf(result, "(StructLiteralExpr%s %s)", type_str, sle->var);
    }
    break;
  case DOTEXPR:;
    dot_expr *de = (dot_expr *)expr->node;
    char *de_expr = expr_to_str(de->expr);
    result = safe_realloc_str(result, BUFSIZ + 1 + strlen(result) +
                                          strlen(type_str) + strlen(de_expr) +
                                          strlen(de->var));
    sprintf(result, "(DotExpr%s %s %s)", type_str, de_expr, de->var);
    free(de_expr);
    break;
  case ARRAYINDEXEXPR:;
    array_index_expr *aie = (array_index_expr *)expr->node;
    char *aie_expr = expr_to_str(aie->expr);

    if (aie->exprs->size != 0) {
      char *aie_list = vector_to_str(aie->exprs);
      result = safe_realloc_str(result,
                                BUFSIZ + 1 + strlen(result) + strlen(type_str) +
                                    strlen(aie_expr) + strlen(aie_list));
      sprintf(result, "(ArrayIndexExpr%s %s %s)", type_str, aie_expr, aie_list);
      free(aie_list);
    } else {
      result =
          safe_realloc_str(result, BUFSIZ + 1 + strlen(result) +
                                       strlen(type_str) + strlen(aie_expr));
      sprintf(result, "(ArrayIndexExpr%s %s)", type_str, aie_expr);
    }
    free(aie_expr);
    break;
  case CALLEXPR:;
    call_expr *ce = (call_expr *)expr->node;
    if (ce->exprs->size != 0) {
      char *ce_list = vector_to_str(ce->exprs);
      result = safe_realloc_str(result, BUFSIZ + 1 + strlen(result) +
                                            strlen(type_str) + strlen(ce->var) +
                                            strlen(ce_list));
      sprintf(result, "(CallExpr%s %s %s)", type_str, ce->var, ce_list);
      free(ce_list);
    } else {
      result = safe_realloc_str(result, BUFSIZ + 1 + strlen(result) +
                                            strlen(type_str) + strlen(ce->var));
      sprintf(result, "(CallExpr%s %s)", type_str, ce->var);
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
    result = safe_realloc_str(result, BUFSIZ + 1 + strlen(result) +
                                          strlen(type_str) + strlen(u_op) +
                                          strlen(u_rhs));
    sprintf(result, "(UnopExpr%s %s %s)", type_str, u_op, u_rhs);
    free(u_rhs);
    break;
  case BINOPEXPR:;
    binop_expr *be = (binop_expr *)expr->node;
    char *b_op = bop_to_str(be->op);
    char *b_lhs = expr_to_str(be->lhs);
    char *b_rhs = expr_to_str(be->rhs);
    result = safe_realloc_str(result, BUFSIZ + 1 + strlen(result) +
                                          strlen(type_str) + strlen(b_lhs) +
                                          strlen(b_op) + strlen(b_rhs));
    sprintf(result, "(BinopExpr%s %s %s %s)", type_str, b_lhs, b_op, b_rhs);
    free(b_lhs);
    free(b_rhs);
    break;
  case IFEXPR:;
    if_expr *ife = (if_expr *)expr->node;
    char *if_e = expr_to_str(ife->if_expr);
    char *then_e = expr_to_str(ife->then_expr);
    char *else_e = expr_to_str(ife->else_expr);
    result = safe_realloc_str(result, BUFSIZ + 1 + strlen(result) +
                                          strlen(type_str) + strlen(if_e) +
                                          strlen(then_e) + strlen(else_e));
    sprintf(result, "(IfExpr%s %s %s %s)", type_str, if_e, then_e, else_e);
    free(if_e);
    free(else_e);
    free(then_e);
    break;
  case ARRAYLOOPEXPR:;
    array_loop_expr *aloop = (array_loop_expr *)expr->node;
    result = safe_realloc_str(result,
                              BUFSIZ + 1 + strlen(result) + strlen(type_str));
    sprintf(result, "(ArrayLoopExpr%s", type_str);
    if (aloop->vars->size != 0) {
      for (int i = 0; i < aloop->vars->size; i++) {
        result = safe_strcat(result, " ");
        result = safe_strcat(result, vector_get_str(aloop->vars, i));
        result = safe_strcat(result, " ");

        char *cur_e = expr_to_str(vector_get_expr(aloop->exprs, i));
        result = safe_strcat(result, cur_e);
        free(cur_e);
      }
    }
    char *a_final_e = expr_to_str(aloop->expr);
    result = safe_strcat(result, " ");
    result = safe_strcat(result, a_final_e);
    result = safe_strcat(result, ")");
    free(a_final_e);
    break;
  case SUMLOOPEXPR:;
    sum_loop_expr *sloop = (sum_loop_expr *)expr->node;
    result = safe_realloc_str(result,
                              BUFSIZ + 1 + strlen(result) + strlen(type_str));
    sprintf(result, "(SumLoopExpr%s", type_str);
    if (sloop->vars->size != 0) {
      for (int i = 0; i < sloop->vars->size; i++) {
        result = safe_strcat(result, " ");
        result = safe_strcat(result, vector_get_str(sloop->vars, i));
        result = safe_strcat(result, " ");

        char *cur_e = expr_to_str(vector_get_expr(sloop->exprs, i));
        result = safe_strcat(result, cur_e);
        free(cur_e);
      }
    }
    char *s_final_e = expr_to_str(sloop->expr);
    result = safe_strcat(result, " ");
    result = safe_strcat(result, s_final_e);
    result = safe_strcat(result, ")");
    free(s_final_e);
    break;
  }

  free(type_str);
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
  char *result = safe_alloc(BUFSIZ);
  switch (lval->type) {
  case VARLVALUE:;
    var_lval *vlv = (var_lval *)lval->node;
    result = safe_realloc_str(result,
                              BUFSIZ + 1 + strlen(result) + strlen(vlv->var));
    sprintf(result, "(VarLValue %s)", vlv->var);
    break;
  case ARRAYLVALUE:;
    array_lval *alv = (array_lval *)lval->node;
    if (alv->vars->size != 0) {
      char *alv_vars = vector_to_str(alv->vars);
      result =
          safe_realloc_str(result, BUFSIZ + 1 + strlen(result) +
                                       strlen(alv->var) + strlen(alv_vars));
      sprintf(result, "(ArrayLValue %s %s)", alv->var, alv_vars);
      free(alv_vars);
    } else {
      result = safe_realloc_str(result,
                                BUFSIZ + 1 + strlen(result) + strlen(alv->var));
      sprintf(result, "(ArrayLValue %s)", alv->var);
    }
    break;
  }
  return result;
}

char *stmt_to_str(stmt *stmt) {
  char *result = safe_alloc(BUFSIZ);
  switch (stmt->type) {
  case LETSTMT:;
    let_stmt *ls = (let_stmt *)stmt->node;
    char *ls_lval = lval_to_str(ls->lval);
    char *ls_expr = expr_to_str(ls->expr);
    result = safe_realloc_str(result, BUFSIZ + 1 + strlen(result) +
                                          strlen(ls_lval) + strlen(ls_expr));
    sprintf(result, "(LetStmt %s %s)", ls_lval, ls_expr);
    free(ls_lval);
    free(ls_expr);
    break;
  case ASSERTSTMT:;
    assert_stmt *as = (assert_stmt *)stmt->node;
    char *as_expr = expr_to_str(as->expr);
    result = safe_realloc_str(result, BUFSIZ + 1 + strlen(result) +
                                          strlen(as_expr) + strlen(as->str));
    sprintf(result, "(AssertStmt %s %s)", as_expr, as->str);
    free(as_expr);
    break;
  case RETURNSTMT:;
    return_stmt *rs = (return_stmt *)stmt->node;
    char *rs_expr = expr_to_str(rs->expr);
    result =
        safe_realloc_str(result, BUFSIZ + 1 + strlen(result) + strlen(rs_expr));
    sprintf(result, "(ReturnStmt %s)", rs_expr);
    free(rs_expr);
    break;
  }
  return result;
}

char *type_to_str(type *type) {
  char *result = safe_alloc(BUFSIZ);
  switch (type->type) {
  case INTTYPE:
    sprintf(result, "(IntType)");
    break;
  case FLOATTYPE:
    sprintf(result, "(FloatType)");
    break;
  case BOOLTYPE:
    sprintf(result, "(BoolType)");
    break;
  case ARRAYTYPE:;
    array_type *at = (array_type *)type->node;
    char *at_t = type_to_str(at->type);
    result = safe_realloc_str(result,
                              BUFSIZ + 1 + strlen(result) + strlen(at_t) + 12);
    sprintf(result, "(ArrayType %s %d)", at_t, at->rank);
    free(at);
    break;
  case STRUCTTYPE:;
    struct_type *st = (struct_type *)type->node;
    result =
        safe_realloc_str(result, BUFSIZ + 1 + strlen(result) + strlen(st->var));
    sprintf(result, "(StructType %s)", st->var);
    break;
  case VOIDTYPE:
    sprintf(result, "(VoidType)");
    break;
  }
  return result;
}

char *vector_to_str(vector *v) {
  char *result = NULL;
  long len = 0;

  for (int i = 0; i < v->size; i++) {
    char *cur = NULL;
    switch (v->type) {
    case CMDVECTOR:;
      cur = cmd_to_str(vector_get_cmd(v, i));
      break;
    case EXPRVECTOR:;
      cur = expr_to_str(vector_get_expr(v, i));
      break;
    case STMTVECTOR:;
      cur = stmt_to_str(vector_get_stmt(v, i));
      break;
    case TYPEVECTOR:;
      cur = type_to_str(vector_get_type(v, i));
      break;
    case STRVECTOR:;
      char *cur_var = vector_get_str(v, i);
      cur = safe_alloc(strlen(cur_var) + 1);
      memcpy(cur, cur_var, strlen(cur_var));
      break;
    default:
      return NULL;
    }

    char *tmp = NULL;
    while (tmp == NULL)
      tmp = safe_realloc_str(result, BUFSIZ + 1 + len + strlen(cur) + 2);
    result = tmp;
    memset(result + len, 0, strlen(cur) + 2);
    len += strlen(cur) + 2;

    result = safe_strcat(result, cur);
    free(cur);
    if (i != v->size - 1)
      result = safe_strcat(result, " ");
  }

  return result;
}

char *t_to_str(t *t) {
  if (t == NULL)
    return NULL;

  char *result = safe_alloc(BUFSIZ);
  switch (t->type) {
  case INT_T:
    sprintf(result, "(IntType)");
    break;
  case FLOAT_T:
    sprintf(result, "(FloatType)");
    break;
  case BOOL_T:
    sprintf(result, "(BoolType)");
    break;
  case VOID_T:
    sprintf(result, "(VoidType)");
    break;
  case ARRAY_T:;
    array_info *a_info = (array_info *)t->info;
    char *a_type = t_to_str(a_info->type);
    result = safe_realloc_str(result, BUFSIZ + 1 + strlen(result) +
                                          strlen(a_type) + 12);
    sprintf(result, "(ArrayType %s %d)", a_type, a_info->rank);
    free(a_type);
    break;
  case STRUCT_T:;
    struct_info *s_info = (struct_info *)t->info;
    result = safe_realloc_str(result, BUFSIZ + 1 + strlen(result) +
                                          strlen(s_info->name));
    sprintf(result, "(StructType %s)", s_info->name);
    break;
  case FN_T:
    // TODO HW7
    sprintf(result, "(FnType)");
    break;
  default:
    return NULL;
  }

  return result;
}
