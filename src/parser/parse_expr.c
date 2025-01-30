#include "parse_expr.h"
#include "alloc.h"
#include "ast.h"
#include "compiler_error.h"
#include "parser.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parse_expr(Vector *tokens, int i, Expr *e) {
  e->start = i;
  int type = peek_token(tokens, i);

  switch (type) {
  case INTVAL:;
    IntExpr *ie = alloc(sizeof(IntExpr));
    ie->start = i;

    char *ie_str = vector_get_token(tokens, i)->text;
    ie->val = strtol(ie_str, NULL, 10);
    if (errno == ERANGE) {
      char *msg = alloc(BUFSIZ);
      sprintf(msg, "Int '%s' out of range", ie_str);
      parse_error(msg);
    }

    i += 1;
    e->node = ie;
    e->type = INTEXPR;
    break;
  case FLOATVAL:;
    FloatExpr *fe = alloc(sizeof(FloatExpr));
    fe->start = i;

    char *fe_str = vector_get_token(tokens, i)->text;
    fe->val = strtod(fe_str, NULL);
    if (errno == ERANGE) {
      char *msg = alloc(BUFSIZ);
      sprintf(msg, "Int '%s' out of range", fe_str);
      parse_error(msg);
    }

    i += 1;
    e->node = fe;
    e->type = FLOATEXPR;
    break;
  case TRUE:;
    TrueExpr *te = alloc(sizeof(TrueExpr));
    te->start = i;
    e->node = te;
    e->type = TRUEEXPR;
    i += 1;
    break;
  case FALSE:;
    FalseExpr *fae = alloc(sizeof(FalseExpr));
    fae->start = i;
    e->node = fae;
    e->type = FALSEEXPR;
    i += 1;
    break;
  case VARIABLE:;
    char *v_str = vector_get_token(tokens, i)->text;
    char *v_var = alloc(strlen(v_str) + 1);
    memcpy(v_var, v_str, strlen(v_str));

    switch (peek_token(tokens, i + 1)) {
    case LCURLY:;
      StructLiteralExpr *sle = alloc(sizeof(StructLiteralExpr));
      sle->start = i;
      sle->var = v_var;
      i += 2;

      sle->list = NULL;
      if (peek_token(tokens, i) != RCURLY) {
        sle->list = alloc(sizeof(ExprList));
        i = parse_expr_list(tokens, i, sle->list);
        expect_token(tokens, i, RCURLY);
      }

      e->type = STRUCTLITERALEXPR;
      e->node = sle;
      i += 1;
      break;
    case LPAREN:;
      CallExpr *ce = alloc(sizeof(CallExpr));
      ce->start = i;
      ce->var = v_var;
      i += 2;

      ce->list = NULL;
      if (peek_token(tokens, i) != RPAREN) {
        ce->list = alloc(sizeof(ExprList));
        i = parse_expr_list(tokens, i, ce->list);
        expect_token(tokens, i, RPAREN);
      }

      e->type = CALLEXPR;
      e->node = ce;
      i += 1;
      break;
    default:;
      VarExpr *ve = alloc(sizeof(VarExpr));
      ve->start = i;
      ve->var = v_var;
      e->node = ve;
      e->type = VAREXPR;
      i += 1;
    }
    break;
  case LPAREN:;
    i += 1;
    Expr *new_e = alloc(sizeof(Expr));
    i = parse_expr(tokens, i, new_e);
    expect_token(tokens, i, RPAREN);

    e->type = EXPR;
    e->node = new_e;
    i += 1;
    break;
  case LSQUARE:;
    ArrayLiteralExpr *ale = alloc(sizeof(ArrayLiteralExpr));
    ale->start = i;
    i += 1;

    ale->list = NULL;
    if (peek_token(tokens, i) != RSQUARE) {
      ale->list = alloc(sizeof(ExprList));
      i = parse_expr_list(tokens, i, ale->list);

      expect_token(tokens, i, RSQUARE);
    }

    e->node = ale;
    e->type = ARRAYLITERALEXPR;
    i += 1;
    break;
  case VOID:;
    VoidExpr *vde = alloc(sizeof(VoidExpr));
    vde->start = i;
    e->type = VOIDEXPR;
    e->node = vde;
    i += 1;
    break;
  default:;
    char *msg = alloc(BUFSIZ);
    sprintf(msg, "Unexpected token '%s' at %d",
            vector_get_token(tokens, i)->text, i);
    parse_error(msg);
  }

  // Check dot and array index
  i = parse_expr_cont(tokens, i, e);

  return i;
}

int parse_expr_cont(Vector *tokens, int i, Expr *e) {
  int type = peek_token(tokens, i);
  if (type != DOT && type != LSQUARE)
    return i;
  if (type == DOT) {
    DotExpr *de = alloc(sizeof(DotExpr));
    de->start = e->start;
    i += 1;

    expect_token(tokens, i, VARIABLE);
    char *de_str = vector_get_token(tokens, i)->text;
    de->var = alloc(strlen(de_str) + 1);
    memcpy(de->var, de_str, strlen(de_str));

    Expr *old_e = alloc(sizeof(Expr));
    old_e->start = e->start;
    old_e->type = e->type;
    old_e->node = e->node;
    de->expr = old_e;

    e->type = DOTEXPR;
    e->node = de;
    i += 1;
  } else if (type == LSQUARE) {
    ArrayIndexExpr *aie = alloc(sizeof(ArrayIndexExpr));
    aie->start = aie->start;
    aie->expr = e->node;
    i += 1;

    aie->list = NULL;
    if (peek_token(tokens, i) != RSQUARE) {
      aie->list = alloc(sizeof(ExprList));
      i = parse_expr_list(tokens, i, aie->list);
    }
    expect_token(tokens, i, RSQUARE);

    Expr *old_e = alloc(sizeof(Expr));
    old_e->start = e->start;
    old_e->type = e->type;
    old_e->node = e->node;
    aie->expr = old_e;

    e->type = ARRAYINDEXEXPR;
    e->node = aie;
    i += 1;
  }

  i = parse_expr_cont(tokens, i, e);
  return i;
}

int parse_expr_list(Vector *tokens, int i, ExprList *list) {
  Vector *nodes = alloc(sizeof(Vector));
  vector_init(nodes, 8, EXPRVECTOR);

  while (i < tokens->size - 1) {
    Expr *e = alloc(sizeof(Expr));
    e->start = i;
    i = parse_expr(tokens, i, e);
    vector_append(nodes, e);

    if (peek_token(tokens, i) != COMMA) {
      list->exprs_size = nodes->size;
      list->exprs = (Expr **)nodes->data;
      break;
    }

    i += 1;
  }

  return i;
}
