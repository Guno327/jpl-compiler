#include "parse_expr.h"
#include "alloc.h"
#include "ast.h"
#include "compiler_error.h"
#include "parser.h"
#include "token.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int parse_expr(Vector *tokens, int i, Expr *e) {
  // Setup Expr to be empty
  e->start = i;
  e->node = NULL;
  e->type = 0;

  i = parse_base_level(tokens, i, e);
  i = parse_index_level(tokens, i, e);

  if (e->node == NULL) {
    char *msg = alloc(BUFSIZ);
    sprintf(msg, "Unexpected token '%s' at %d",
            vector_get_token(tokens, i)->text, i);
  }

  return i;
}

int parse_base_level(Vector *tokens, int i, Expr *e) {
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
  case IF:;
    IfExpr *ife = alloc(sizeof(IfExpr));
    ife->start = i;
    i += 1;

    ife->if_expr = alloc(sizeof(Expr));
    i = parse_expr(tokens, i, ife->if_expr);

    expect_token(tokens, i, ELSE);
    i += 1;

    ife->then_expr = alloc(sizeof(Expr));
    i = parse_expr(tokens, i, ife->then_expr);

    expect_token(tokens, i, ELSE);
    i += 1;

    ife->else_expr = alloc(sizeof(Expr));
    i = parse_expr(tokens, i, ife->else_expr);

    e->type = IFEXPR;
    e->node = ife;
    break;
  case ARRAY:;
    ArrayLoopExpr *aloop = alloc(sizeof(ArrayLoopExpr));
    aloop->start = i;
    expect_token(tokens, i += 1, LSQUARE);
    i += 1;

    Vector *a_vars = alloc(sizeof(Vector));
    Vector *a_exprs = alloc(sizeof(Vector));
    vector_init(a_vars, 8, STRVECTOR);
    vector_init(a_exprs, 8, EXPRVECTOR);
    while (i < tokens->size) {
      expect_token(tokens, i, VARIABLE);
      char *a_cur_str = vector_get_token(tokens, i)->text;
      char *a_cur_var = alloc(strlen(a_cur_str) + 1);
      memcpy(a_cur_var, a_cur_str, strlen(a_cur_str));
      vector_append(a_vars, a_cur_var);
      i += 1;

      expect_token(tokens, i, COLON);
      i += 1;

      Expr *a_cur_e = alloc(sizeof(Expr));
      i = parse_expr(tokens, i, a_cur_e);
      vector_append(a_exprs, a_cur_e);

      if (peek_token(tokens, i) != COMMA)
        break;
      i += 1;
    }
    expect_token(tokens, i, RSQUARE);
    i += 1;

    aloop->vars_size = a_vars->size;
    aloop->vars = (char **)a_vars->data;

    aloop->list = alloc(sizeof(ExprList));
    aloop->list->exprs_size = a_exprs->size;
    aloop->list->exprs = (Expr **)a_exprs->data;

    aloop->expr = alloc(sizeof(Expr));
    i = parse_expr(tokens, i, aloop->expr);

    e->type = ARRAYLOOPEXPR;
    e->node = aloop;
    break;
  case SUM:;
    SumLoopExpr *sloop = alloc(sizeof(SumLoopExpr));
    sloop->start = i;
    expect_token(tokens, i += 1, LSQUARE);
    i += 1;

    Vector *s_vars = alloc(sizeof(Vector));
    Vector *s_exprs = alloc(sizeof(Vector));
    vector_init(s_vars, 8, STRVECTOR);
    vector_init(s_exprs, 8, EXPRVECTOR);
    while (i < tokens->size) {
      expect_token(tokens, i, VARIABLE);
      char *s_cur_str = vector_get_token(tokens, i)->text;
      char *s_cur_var = alloc(strlen(s_cur_str) + 1);
      memcpy(s_cur_var, s_cur_str, strlen(s_cur_str));
      vector_append(s_vars, s_cur_var);
      i += 1;

      expect_token(tokens, i, COLON);
      i += 1;

      Expr *s_cur_e = alloc(sizeof(Expr));
      i = parse_expr(tokens, i, s_cur_e);
      vector_append(s_exprs, s_cur_e);

      if (peek_token(tokens, i) != ',')
        break;
      i += 1;
    }
    expect_token(tokens, i, RSQUARE);
    i += 1;

    sloop->vars_size = s_vars->size;
    sloop->vars = (char **)s_vars->data;

    sloop->list = alloc(sizeof(ExprList));
    sloop->list->exprs_size = s_exprs->size;
    sloop->list->exprs = (Expr **)s_exprs->data;

    sloop->expr = alloc(sizeof(Expr));
    i = parse_expr(tokens, i, sloop->expr);

    e->type = SUMLOOPEXPR;
    e->node = sloop;
    break;
  default:
    break;
  }

  return i;
}

int parse_index_level(Vector *tokens, int i, Expr *e) {
  // If DOT or LSQUARE we have index
  int type = peek_token(tokens, i);
  // DotExpr
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
  }
  // ArrayIndexExpr
  else if (type == LSQUARE) {
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
  // Not index, must move deeper
  else {
    int old_i = i;
    i = parse_unary_level(tokens, i, e);
    if (old_i == i)
      return i;
  }

  i = parse_index_level(tokens, i, e);
  return i;
}

int parse_unary_level(Vector *tokens, int i, Expr *e) {
  // If OP we might have unary
  TokenType type = peek_token(tokens, i);
  Token *t = vector_get_token(tokens, i);
  // Unary not
  if (type == OP && !strcmp(t->text, "!")) {
    // e must be empty for this to be valid
    if (e->node != NULL) {
      char *msg = alloc(BUFSIZ);
      sprintf("Unexpected token '%s' at %d", vector_get_token(tokens, i)->text,
              i);
      parse_error(msg);
    }

    UnopExpr *not_ue = alloc(sizeof(UnopExpr));
    not_ue->start = i;
    not_ue->op = NOTOP;
    i += 1;

    not_ue->rhs = alloc(sizeof(Expr));
    i = parse_mult_level(tokens, i, not_ue->rhs);

    e->type = UNOPEXPR;
    e->start = not_ue->start;
    e->node = not_ue;
  }
  // Unary negation
  else if (type == OP && !strcmp(t->text, "-")) {
    // e must be empty for this to be valid
    if (e->node != NULL) {
      char *msg = alloc(BUFSIZ);
      sprintf("Unexpected token '%s' at %d", vector_get_token(tokens, i)->text,
              i);
      parse_error(msg);
    }

    UnopExpr *neg_ue = alloc(sizeof(UnopExpr));
    neg_ue->start = i;
    neg_ue->op = NEGOP;
    i += 1;

    neg_ue->rhs = alloc(sizeof(Expr));
    i = parse_mult_level(tokens, i, neg_ue->rhs);

    e->type = UNOPEXPR;
    e->start = neg_ue->start;
    e->node = neg_ue;
  }
  // Not unary, must go deeper
  else {
    int old_i = i;
    i = parse_mult_level(tokens, i, e);
    if (old_i == i)
      return i;
  }

  i = parse_unary_level(tokens, i, e);
  return i;
}

int parse_mult_level(Vector *tokens, int i, Expr *e) {
  int type = peek_token(tokens, i);
  // Not mult, move deeper
  if (type != OP) {
    int old_i = i;
    i = parse_add_level(tokens, i, e);
    if (old_i == i)
      return i;
  } else {
    Token *t = vector_get_token(tokens, i);
    char c = t->text[0];
    if (c == '*' || c == '%' || c == '/') {
      BinopExpr *be = alloc(sizeof(BinopExpr));
      be->start = i;
      i += 1;

      Expr *old_e = alloc(sizeof(Expr));
      old_e->start = e->start;
      old_e->type = e->type;
      old_e->node = e->node;
      be->lhs = old_e;

      be->rhs = alloc(sizeof(Expr));
      i = parse_add_level(tokens, i, be->rhs);

      switch (c) {
      case '*':
        be->op = MULTOP;
        break;
      case '/':
        be->op = DIVOP;
        break;
      case '%':
        be->op = MODOP;
        break;
      }

      e->type = BINOPEXPR;
      e->start = be->start;
      e->node = be;
    }
    // Not mult, we need to go deeper
    else {
      int old_i = i;
      i = parse_add_level(tokens, i, e);
      if (old_i == i)
        return i;
    }
  }

  // Recurse at this level
  i = parse_mult_level(tokens, i, e);
  return i;
}

int parse_add_level(Vector *tokens, int i, Expr *e) {
  int type = peek_token(tokens, i);
  // Not add, move deeper
  if (type != OP) {
    int old_i = i;
    i = parse_cmp_level(tokens, i, e);
    if (old_i == i)
      return i;
  } else {
    Token *t = vector_get_token(tokens, i);
    char c = t->text[0];
    if (c == '+' || c == '-') {
      BinopExpr *be = alloc(sizeof(BinopExpr));
      be->start = i;
      i += 1;

      Expr *old_e = alloc(sizeof(Expr));
      old_e->start = e->start;
      old_e->type = e->type;
      old_e->node = e->node;
      be->lhs = old_e;

      be->rhs = alloc(sizeof(Expr));
      i = parse_cmp_level(tokens, i, be->rhs);

      switch (c) {
      case '+':
        be->op = ADDOP;
        break;
      case '-':
        be->op = SUBOP;
        break;
      }

      e->type = BINOPEXPR;
      e->start = be->start;
      e->node = be;
    }
    // Not mult, we need to go deeper
    else {
      int old_i = i;
      i = parse_cmp_level(tokens, i, e);
      if (old_i == i)
        return i;
    }
  }

  // Recurse at this level
  i = parse_add_level(tokens, i, e);
  return i;
}

int parse_cmp_level(Vector *tokens, int i, Expr *e) {
  int type = peek_token(tokens, i);
  // Not cmp, move deeper
  if (type != OP) {
    int old_i = i;
    i = parse_bool_level(tokens, i, e);
    if (old_i == i)
      return i;
  } else {
    Token *t = vector_get_token(tokens, i);
    char c = t->text[0];
    if (c == '<' || c == '>' || c == '=' || c == '!') {
      BinopExpr *be = alloc(sizeof(BinopExpr));
      be->start = i;
      i += 1;

      Expr *old_e = alloc(sizeof(Expr));
      old_e->start = e->start;
      old_e->type = e->type;
      old_e->node = e->node;
      be->lhs = old_e;

      be->rhs = alloc(sizeof(Expr));
      i = parse_bool_level(tokens, i, be->rhs);

      switch (c) {
      case '<':
        if (strlen(t->text) == 1)
          be->op = LTOP;
        else if (t->text[1] == '=')
          be->op = LEOP;
        else {
          char *msg = alloc(BUFSIZ);
          sprintf("Unexpected char '%c' in token '%s'", &t->text[1], t->text);
          parse_error(msg);
        }
        break;
      case '>':
        if (strlen(t->text) == 1)
          be->op = GTOP;
        else if (t->text[1] == '=')
          be->op = GEOP;
        else {
          char *msg = alloc(BUFSIZ);
          sprintf("Unexpected char '%c' in token '%s'", &t->text[1], t->text);
          parse_error(msg);
        }
        break;
      case '=':
        if (strlen(t->text) > 1 && t->text[1] == '=')
          be->op = EQOP;
        else {
          char *msg = alloc(BUFSIZ);
          sprintf("Unexpected char '%c' in token '%s'", &t->text[1], t->text);
          parse_error(msg);
        }
        break;
      case '!':
        if (strlen(t->text) > 1 && t->text[1] == '=')
          be->op = NEOP;
        else {
          char *msg = alloc(BUFSIZ);
          sprintf("Unexpected char '%c' in token '%s'", &t->text[1], t->text);
          parse_error(msg);
        }
        break;
      }

      e->type = BINOPEXPR;
      e->start = be->start;
      e->node = be;
    }
    // Not mult, we need to go deeper
    else {
      int old_i = i;
      i = parse_bool_level(tokens, i, e);
      if (old_i == i)
        return i;
    }
  }

  // Recurse at this level
  i = parse_cmp_level(tokens, i, e);
  return i;
}

int parse_bool_level(Vector *tokens, int i, Expr *e) {
  int type = peek_token(tokens, i);
  // Not bool, move deeper
  if (type != OP) {
    int old_i = i;
    i = parse_base_level(tokens, i, e);
    if (old_i == i)
      return i;
  } else {
    Token *t = vector_get_token(tokens, i);
    char c = t->text[0];
    if (c == '&' || c == '|') {
      BinopExpr *be = alloc(sizeof(BinopExpr));
      be->start = i;
      i += 1;

      Expr *old_e = alloc(sizeof(Expr));
      old_e->start = e->start;
      old_e->type = e->type;
      old_e->node = e->node;
      be->lhs = old_e;

      be->rhs = alloc(sizeof(Expr));
      i = parse_base_level(tokens, i, be->rhs);

      switch (c) {
      case '&':
        if (strlen(t->text) > 1 && t->text[1] == '&')
          be->op = ANDOP;
        else {
          char *msg = alloc(BUFSIZ);
          sprintf("Unexpected char '%c' in token '%s'", &t->text[1], t->text);
          parse_error(msg);
        }
        break;
      case '|':
        if (strlen(t->text) > 1 && t->text[1] == '|')
          be->op = OROP;
        else {
          char *msg = alloc(BUFSIZ);
          sprintf("Unexpected char '%c' in token '%s'", &t->text[1], t->text);
          parse_error(msg);
        }
        break;
      }

      e->type = BINOPEXPR;
      e->start = be->start;
      e->node = be;
    }
    // Not mult, we need to go deeper
    else {
      int old_i = i;
      i = parse_base_level(tokens, i, e);
      if (old_i == i)
        return i;
    }
  }

  // Recurse at this level
  i = parse_bool_level(tokens, i, e);
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
