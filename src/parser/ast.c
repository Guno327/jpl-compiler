#include "ast.h"
#include "alloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *print_cmd(Cmd *cmd) {
  char *result = alloc(BUFSIZ);
  switch (cmd->type) {
  case READCMD:;
    ReadCmd *rc = (ReadCmd *)cmd->node;
    char *rc_lval = print_lvalue(rc->lval);
    sprintf(result, "(ReadCmd %s %s)", rc->str, rc_lval);
    free(rc_lval);
    break;
  case WRITECMD:;
    WriteCmd *wc = (WriteCmd *)cmd->node;
    char *wc_expr = print_expr(wc->expr);
    sprintf(result, "(WriteCmd %s %s)", wc_expr, wc->str);
    free(wc_expr);
    break;
  case LETCMD:;
    LetCmd *lc = (LetCmd *)cmd->node;
    char *lc_lval = print_lvalue(lc->lval);
    char *lc_expr = print_expr(lc->expr);
    sprintf(result, "(LetCmd %s %s)", lc_lval, lc_expr);
    free(lc_lval);
    free(lc_expr);
    break;
  case ASSERTCMD:;
    AssertCmd *ac = (AssertCmd *)cmd->node;
    char *ac_expr = print_expr(ac->expr);
    sprintf(result, "(AssertCmd %s %s)", ac_expr, ac->str);
    break;
  case PRINTCMD:;
    PrintCmd *pc = (PrintCmd *)cmd->node;
    sprintf(result, "(PrintCmd %s)", pc->str);
    break;
  case SHOWCMD:;
    ShowCmd *sc = (ShowCmd *)cmd->node;
    char *sc_expr = print_expr(sc->expr);
    sprintf(result, "(ShowCmd %s)", sc_expr);
    free(sc_expr);
    break;
  case TIMECMD:;
    TimeCmd *tc = (TimeCmd *)cmd->node;
    char *tc_cmd = print_cmd(tc->cmd);
    sprintf(result, "(TimeCmd %s)", tc_cmd);
    free(tc_cmd);
    break;
  case FNCMD:;
    FnCmd *fc = (FnCmd *)cmd->node;

    sprintf(result, "(FnCmd %s ((", fc->var);
    if (fc->binds != NULL) {
      for (int i = 0; i < fc->binds_size; i++) {
        char *cur_lv = print_lvalue(fc->binds[i]->lval);
        char *cur_t = print_type(fc->binds[i]->type);
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

    char *fc_type = print_type(fc->type);
    strcat(result, fc_type);
    strcat(result, " ");
    free(fc_type);

    if (fc->stmts != NULL) {
      char *fc_stmts = print_list((void **)fc->stmts, fc->stmts_size, STMTLIST);
      strcat(result, fc_stmts);
      free(fc_stmts);
    }

    strcat(result, ")");
    break;
  case STRUCTCMD:;
    StructCmd *stc = (StructCmd *)cmd->node;
    sprintf(result, "(StructCmd %s", stc->var);

    if (stc->vars != NULL && stc->types != NULL) {
      for (int i = 0; i < stc->vars_size; i++) {
        char *cur_v = stc->vars[i];
        char *cur_t = print_type(stc->types[i]);
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

char *print_expr(Expr *expr) {
  char *result = alloc(BUFSIZ);
  switch (expr->type) {
  case INTEXPR:;
    IntExpr *ie = (IntExpr *)expr->node;
    sprintf(result, "(IntExpr %lu)", ie->val);
    break;
  case FLOATEXPR:;
    FloatExpr *fe = (FloatExpr *)expr->node;
    sprintf(result, "(FloatExpr %lu)", (long)fe->val);
    break;
  case TRUEEXPR:
    sprintf(result, "(TrueExpr)");
    break;
  case FALSEEXPR:
    sprintf(result, "(FalseExpr)");
    break;
  case VAREXPR:;
    VarExpr *ve = (VarExpr *)expr->node;
    sprintf(result, "(VarExpr %s)", ve->var);
    break;
  case ARRAYLITERALEXPR:;
    ArrayLiteralExpr *ale = (ArrayLiteralExpr *)expr->node;
    if (ale->list != NULL) {
      char *ale_list = print_list((void **)ale->list->exprs,
                                  ale->list->exprs_size, EXPRLIST);
      sprintf(result, "(ArrayLiteralExpr %s)", ale_list);
      free(ale_list);
    } else {
      sprintf(result, "(ArrayLiteralExpr)");
    }
    break;
  case VOIDEXPR:
    sprintf(result, "(VoidExpr)");
    break;
  case STRUCTLITERALEXPR:;
    StructLiteralExpr *sle = (StructLiteralExpr *)expr->node;
    if (sle->list != NULL) {
      char *sle_list = print_list((void **)sle->list->exprs,
                                  sle->list->exprs_size, EXPRLIST);
      sprintf(result, "(StructLiteralExpr %s %s)", sle->var, sle_list);
      free(sle_list);
    } else {
      sprintf(result, "(StructLiteralExpr %s)", sle->var);
    }
    break;
  case DOTEXPR:;
    DotExpr *de = (DotExpr *)expr->node;
    char *de_expr = print_expr(de->expr);
    sprintf(result, "(DotExpr %s %s)", de_expr, de->var);
    free(de_expr);
    break;
  case ARRAYINDEXEXPR:;
    ArrayIndexExpr *aie = (ArrayIndexExpr *)expr->node;
    char *aie_expr = print_expr(aie->expr);

    if (aie->list != NULL) {
      char *aie_list = print_list((void **)aie->list->exprs,
                                  aie->list->exprs_size, EXPRLIST);
      sprintf(result, "(ArrayIndexExpr %s %s)", aie_expr, aie_list);
      free(aie_list);
    } else {
      sprintf(result, "(ArrayIndexExpr %s)", aie_expr);
    }
    free(aie_expr);
    break;
  case CALLEXPR:;
    CallExpr *ce = (CallExpr *)expr->node;
    if (ce->list != NULL) {
      char *ce_list =
          print_list((void **)ce->list->exprs, ce->list->exprs_size, EXPRLIST);
      sprintf(result, "(CallExpr %s %s)", ce->var, ce_list);
      free(ce_list);
    } else {
      sprintf(result, "(CallExpr %s)", ce->var);
    }
    break;
  case EXPR:;
    Expr *inner_e = (Expr *)expr->node;
    result = print_expr(inner_e);
    break;
  case UNOPEXPR:;
    UnopExpr *ue = (UnopExpr *)expr->node;
    char *u_op = print_uop(ue->op);
    char *u_rhs = print_expr(ue->rhs);
    sprintf(result, "(UnopExpr %s %s)", u_op, u_rhs);
    free(u_rhs);
    break;
  case BINOPEXPR:;
    BinopExpr *be = (BinopExpr *)expr->node;
    char *b_op = print_bop(be->op);
    char *b_lhs = print_expr(be->lhs);
    char *b_rhs = print_expr(be->rhs);
    sprintf(result, "(BinopExpr %s %s %s)", b_lhs, b_op, b_rhs);
    free(b_lhs);
    free(b_rhs);
    break;
  case IFEXPR:;
    IfExpr *ife = (IfExpr *)expr->node;
    char *if_e = print_expr(ife->if_expr);
    char *then_e = print_expr(ife->then_expr);
    char *else_e = print_expr(ife->else_expr);
    sprintf(result, "(IfExpr %s %s %s)", if_e, then_e, else_e);
    free(if_e);
    free(else_e);
    free(then_e);
    break;
  case ARRAYLOOPEXPR:;
    ArrayLoopExpr *aloop = (ArrayLoopExpr *)expr->node;
    sprintf(result, "(ArrayLoopExpr");
    for (int i = 0; i < aloop->vars_size; i++) {
      strcat(result, " ");
      strcat(result, aloop->vars[i]);
      strcat(result, " ");

      char *cur_e = print_expr(aloop->list->exprs[i]);
      strcat(result, cur_e);
      free(cur_e);
    }
    char *a_final_e = print_expr(aloop->expr);
    strcat(result, " ");
    strcat(result, a_final_e);
    strcat(result, ")");
    free(a_final_e);
    break;
  case SUMLOOPEXPR:;
    SumLoopExpr *sloop = (SumLoopExpr *)expr->node;
    sprintf(result, "(SumLoopExpr");
    for (int i = 0; i < sloop->vars_size; i++) {
      strcat(result, " ");
      strcat(result, sloop->vars[i]);
      strcat(result, " ");

      char *cur_e = print_expr(sloop->list->exprs[i]);
      strcat(result, cur_e);
      free(cur_e);
    }
    char *s_final_e = print_expr(sloop->expr);
    strcat(result, " ");
    strcat(result, s_final_e);
    strcat(result, ")");
    free(s_final_e);
    break;
  }
  return result;
}

char *print_uop(UnOp op) {
  switch (op) {
  case NOTOP:
    return "!";
  case NEGOP:
    return "-";
  }
  return NULL;
}

char *print_bop(BinOp op) {
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

char *print_lvalue(LValue *lval) {
  char *result = alloc(BUFSIZ);
  switch (lval->type) {
  case VARLVALUE:;
    VarLValue *vlv = (VarLValue *)lval->node;
    sprintf(result, "(VarLValue %s)", vlv->var);
    break;
  case ARRAYLVALUE:;
    ArrayLValue *alv = (ArrayLValue *)lval->node;
    if (alv->vars != NULL) {
      char *alv_vars = print_list((void **)alv->vars, alv->vars_size, VARLIST);
      sprintf(result, "(ArrayLValue %s %s)", alv->var, alv_vars);
      free(alv_vars);
    } else {
      sprintf(result, "(ArrayLValue %s)", alv->var);
    }
    break;
  }
  return result;
}

char *print_stmt(Stmt *stmt) {
  char *result = alloc(BUFSIZ);
  switch (stmt->type) {
  case LETSTMT:;
    LetStmt *ls = (LetStmt *)stmt->node;
    char *ls_lval = print_lvalue(ls->lval);
    char *ls_expr = print_expr(ls->expr);
    sprintf(result, "(LetStmt %s %s)", ls_lval, ls_expr);
    free(ls_lval);
    free(ls_expr);
    break;
  case ASSERTSTMT:;
    AssertStmt *as = (AssertStmt *)stmt->node;
    char *as_expr = print_expr(as->expr);
    sprintf(result, "(AssertStmt %s %s)", as_expr, as->str);
    free(as_expr);
    break;
  case RETURNSTMT:;
    ReturnStmt *rs = (ReturnStmt *)stmt->node;
    char *rs_expr = print_expr(rs->expr);
    sprintf(result, "(ReturnStmt %s)", rs_expr);
    free(rs_expr);
    break;
  }
  return result;
}

char *print_type(Type *type) {
  char *result = alloc(BUFSIZ);
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
    ArrayType *at = (ArrayType *)type->node;
    char *at_t = print_type(at->type);
    sprintf(result, "(ArrayType %s %d)", at_t, at->rank);
    free(at);
    break;
  case STRUCTTYPE:;
    StructType *st = (StructType *)type->node;
    sprintf(result, "(StructType %s)", st->var);
    break;
  case VOIDTYPE:
    sprintf(result, "(VoidType)");
    break;
  }
  return result;
}

char *print_list(void **list, size_t size, ListType type) {
  char *result = NULL;
  size_t len = 0;

  for (int i = 0; i < size; i++) {
    char *cur = NULL;
    switch (type) {
    case CMDLIST:
      cur = print_cmd(((Cmd **)list)[i]);
      break;
    case EXPRLIST:
      cur = print_expr(((Expr **)list)[i]);
      break;
    case LVALUELIST:
      cur = print_lvalue(((LValue **)list)[i]);
      break;
    case STMTLIST:
      cur = print_stmt(((Stmt **)list)[i]);
      break;
    case TYPELIST:
      cur = print_type(((Type **)list)[i]);
      break;
    case VARLIST:;
      char *cur_var = ((char **)list)[i];
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
