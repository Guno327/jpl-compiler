#include "parse_expr.h"
#include "ast.h"
#include "compiler_error.h"
#include "parser.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parse_expr(TokenVector *tokens, int i, Expr *e) {
  e->start = i;
  int type = peek_token(tokens, i);

  switch (type) {
  case INTVAL:;
    IntExpr *ie = malloc(sizeof(IntExpr));
    memset(ie, 0, sizeof(IntExpr));
    ie->start = i;

    char *ie_str = vector_get_token(tokens, i)->text;
    ie->val = strtol(ie_str, NULL, 10);
    if (errno == ERANGE) {
      char *msg = malloc(BUFSIZ);
      sprintf(msg, "Int '%s' out of range", ie_str);
      parse_error(msg);
    }

    i += 1;
    e->node = ie;
    e->type = INTEXPR;
    break;
  case FLOATVAL:;
    FloatExpr *fe = malloc(sizeof(FloatExpr));
    memset(fe, 0, sizeof(FloatExpr));
    fe->start = i;

    char *fe_str = vector_get_token(tokens, i)->text;
    fe->val = strtod(fe_str, NULL);
    if (errno == ERANGE) {
      char *msg = malloc(BUFSIZ);
      sprintf(msg, "Int '%s' out of range", fe_str);
      parse_error(msg);
    }

    i += 1;
    e->node = fe;
    e->type = FLOATEXPR;
    break;
  case TRUE:;
    TrueExpr *te = malloc(sizeof(TrueExpr));
    memset(te, 0, sizeof(TrueExpr));
    te->start = i;
    e->node = te;
    e->type = TRUEEXPR;
    i += 1;
    break;
  case FALSE:;
    FalseExpr *fae = malloc(sizeof(FalseExpr));
    memset(fae, 0, sizeof(FalseExpr));
    fae->start = i;
    e->node = fae;
    e->type = FALSEEXPR;
    i += 1;
    break;
  case VARIABLE:;
    VarExpr *ve = malloc(sizeof(VarExpr));
    memset(ve, 0, sizeof(VarExpr));
    ve->start = i;
    char *ve_var = vector_get_token(tokens, i)->text;
    ve->var = malloc(strlen(ve_var) + 1);
    memset(ve->var, 0, strlen(ve_var) + 1);
    memcpy(ve->var, ve_var, strlen(ve_var));
    e->node = ve;
    e->type = VAREXPR;
    i += 1;
    break;
  case LSQUARE:;
    ArrayLiteralExpr *ale = malloc(sizeof(ArrayLiteralExpr));
    memset(ale, 0, sizeof(ArrayLiteralExpr));
    i = parse_array(tokens, i, ale);
    e->node = ale;
    e->type = ARRAYLITERALEXPR;
    break;
  default:;
    char *msg = malloc(BUFSIZ);
    sprintf(msg, "Unexpected token '%s' at %d",
            vector_get_token(tokens, i)->text, i);
    parse_error(msg);
  }

  return i;
}

int parse_array(TokenVector *tokens, int i, ArrayLiteralExpr *a) {
  ExprVector *nodes = malloc(sizeof(ExprVector));
  memset(nodes, 0, sizeof(ExprVector));
  vector_init_expr(nodes, BUFSIZ);

  expect_token(tokens, i, LSQUARE);
  i += 1;
  while (i < tokens->size - 1) {
    if (peek_token(tokens, i) == RSQUARE) {
      a->exprs = (Expr **)(nodes->data);
      a->exprs_size = nodes->size;
      i += 1;
      break;
    } else {
      Expr *e = malloc(sizeof(Expr));
      memset(e, 0, sizeof(Expr));
      e->start = i;
      i = parse_expr(tokens, i, e);
      vector_append_expr(nodes, e);

      if (peek_token(tokens, i) == RSQUARE) {
        a->exprs = (Expr **)(nodes->data);
        a->exprs_size = nodes->size;
        i += 1;
        break;
      }

      expect_token(tokens, i, COMMA);
      i += 1;
    }
  }

  return i;
}
