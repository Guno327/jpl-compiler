#include "ast.h"
#include "compiler_error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *print_cmd(Cmd *cmd) {
  char *result = malloc(BUFSIZ);
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
  default:
    sprintf(result, "Unexpected CMD in s-print: %d", cmd->type);
    parse_error(result);
  }
  return result;
}

char *print_expr(Expr *expr) {
  char *result = malloc(BUFSIZ);
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
    sprintf(result, "(ArrayLiteralExpr");
    if (ale->list != NULL) {
      Expr **exprs = ale->list->exprs;
      for (int i = 0; i < ale->list->exprs_size; i++) {
        char *expr_str = print_expr(exprs[i]);
        strcat(result, " ");
        strcat(result, expr_str);
        free(expr_str);
      }
    }
    strcat(result, ")");
    break;
  default:
    sprintf(result, "Unexpected EXPR in s-print: %d", expr->type);
    parse_error(result);
  }
  return result;
}

char *print_lvalue(LValue *lval) {
  char *result = malloc(BUFSIZ);
  switch (lval->type) {
  case VARLVALUE:;
    VarLValue *vlv = (VarLValue *)lval->node;
    sprintf(result, "(VarLValue %s)", vlv->var);
    break;
  default:
    sprintf(result, "Unexpected LVALUE in s-print: %d", lval->type);
    parse_error(result);
  }
  return result;
}
