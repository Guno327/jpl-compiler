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

int parse_expr(vector *tokens, int i, expr *e) {
  // Setup expr to be empty
  e->start = i;
  e->node = NULL;
  e->type = 0;
  e->t_type = NULL;

  i = parse_base_level(tokens, i, e);
  if (e->node == NULL)
    parse_error(vector_get_token(tokens, i));
  // Base takes care of unary and index, now we handle the rest
  // we have to start from the bottom up for precedence
  i = parse_bool_level(tokens, i, e);
  if (e->node == NULL)
    parse_error(vector_get_token(tokens, i));

  return i;
}

int parse_base_level(vector *tokens, int i, expr *e) {
  e->start = i;
  int type = peek_token(tokens, i);

  switch (type) {
  case INTVAL:
    if (e->node != NULL)
      parse_error(vector_get_token(tokens, i));
    int_expr *ie = alloc(sizeof(int_expr));
    ie->start = i;

    char *ie_str = vector_get_token(tokens, i)->text;
    ie->val = strtol(ie_str, NULL, 10);
    if (errno == ERANGE) {
      printf("Compilation Failed: Int '%s' at %d out of range", ie_str, i);
      exit(EXIT_FAILURE);
    }

    i += 1;
    e->node = ie;
    e->type = INTEXPR;
    break;
  case FLOATVAL:
    if (e->node != NULL)
      parse_error(vector_get_token(tokens, i));
    float_expr *fe = alloc(sizeof(float_expr));
    fe->start = i;

    char *fe_str = vector_get_token(tokens, i)->text;
    fe->val = strtod(fe_str, NULL);
    if (errno == ERANGE) {
      printf("Compilation Failed: Int '%s' at %d out of range", fe_str, i);
      exit(EXIT_FAILURE);
    }

    i += 1;
    e->node = fe;
    e->type = FLOATEXPR;
    break;
  case TRUE:
    if (e->node != NULL)
      parse_error(vector_get_token(tokens, i));
    true_expr *te = alloc(sizeof(true_expr));
    te->start = i;
    e->node = te;
    e->type = TRUEEXPR;
    i += 1;
    break;
  case FALSE:;
    if (e->node != NULL)
      parse_error(vector_get_token(tokens, i));
    false_expr *fae = alloc(sizeof(false_expr));
    fae->start = i;
    e->node = fae;
    e->type = FALSEEXPR;
    i += 1;
    break;
  case VARIABLE:
    if (e->node != NULL)
      parse_error(vector_get_token(tokens, i));
    char *v_str = vector_get_token(tokens, i)->text;
    char *v_var = alloc(strlen(v_str) + 1);
    memcpy(v_var, v_str, strlen(v_str));

    switch (peek_token(tokens, i + 1)) {
    case LCURLY:;
      struct_literal_expr *sle = alloc(sizeof(struct_literal_expr));
      sle->start = i;
      sle->var = v_var;
      i += 2;

      sle->list = NULL;
      if (peek_token(tokens, i) != RCURLY) {
        sle->list = alloc(sizeof(expr_list));
        i = parse_expr_list(tokens, i, sle->list);
        expect_token(tokens, i, RCURLY);
      }

      e->type = STRUCTLITERALEXPR;
      e->node = sle;
      i += 1;
      break;
    case LPAREN:;
      call_expr *ce = alloc(sizeof(call_expr));
      ce->start = i;
      ce->var = v_var;
      i += 2;

      ce->list = NULL;
      if (peek_token(tokens, i) != RPAREN) {
        ce->list = alloc(sizeof(expr_list));
        i = parse_expr_list(tokens, i, ce->list);
        expect_token(tokens, i, RPAREN);
      }

      e->type = CALLEXPR;
      e->node = ce;
      i += 1;
      break;
    default:;
      var_expr *ve = alloc(sizeof(var_expr));
      ve->start = i;
      ve->var = v_var;
      e->node = ve;
      e->type = VAREXPR;
      i += 1;
    }
    break;
  case LPAREN:
    if (e->node != NULL)
      parse_error(vector_get_token(tokens, i));
    i += 1;
    expr *new_e = alloc(sizeof(expr));
    i = parse_expr(tokens, i, new_e);
    expect_token(tokens, i, RPAREN);

    e->type = EXPR;
    e->node = new_e;
    i += 1;
    break;
  case LSQUARE:
    if (e->node != NULL)
      parse_error(vector_get_token(tokens, i));
    array_literal_expr *ale = alloc(sizeof(array_literal_expr));
    ale->start = i;
    i += 1;

    ale->list = NULL;
    if (peek_token(tokens, i) != RSQUARE) {
      ale->list = alloc(sizeof(expr_list));
      i = parse_expr_list(tokens, i, ale->list);

      expect_token(tokens, i, RSQUARE);
    }

    e->node = ale;
    e->type = ARRAYLITERALEXPR;
    i += 1;
    break;
  case VOID:
    if (e->node != NULL)
      parse_error(vector_get_token(tokens, i));
    void_expr *vde = alloc(sizeof(void_expr));
    vde->start = i;
    e->type = VOIDEXPR;
    e->node = vde;
    i += 1;
    break;
  case IF:
    if (e->node != NULL)
      parse_error(vector_get_token(tokens, i));
    if_expr *ife = alloc(sizeof(if_expr));
    ife->start = i;
    i += 1;

    ife->if_expr = alloc(sizeof(expr));
    i = parse_expr(tokens, i, ife->if_expr);

    expect_token(tokens, i, THEN);
    i += 1;

    ife->then_expr = alloc(sizeof(expr));
    i = parse_expr(tokens, i, ife->then_expr);

    expect_token(tokens, i, ELSE);
    i += 1;

    ife->else_expr = alloc(sizeof(expr));
    i = parse_expr(tokens, i, ife->else_expr);

    e->type = IFEXPR;
    e->node = ife;
    break;
  case ARRAY:
    if (e->node != NULL)
      parse_error(vector_get_token(tokens, i));
    array_loop_expr *aloop = alloc(sizeof(array_loop_expr));
    aloop->start = i;
    expect_token(tokens, i += 1, LSQUARE);
    i += 1;

    vector *a_vars = alloc(sizeof(vector));
    vector *a_exprs = alloc(sizeof(vector));
    vector_init(a_vars, 8, STRVECTOR);
    vector_init(a_exprs, 8, EXPRVECTOR);
    while (i < tokens->size) {
      if (peek_token(tokens, i) == RSQUARE)
        break;
      expect_token(tokens, i, VARIABLE);
      char *a_cur_str = vector_get_token(tokens, i)->text;
      char *a_cur_var = alloc(strlen(a_cur_str) + 1);
      memcpy(a_cur_var, a_cur_str, strlen(a_cur_str));
      vector_append(a_vars, a_cur_var);
      i += 1;

      expect_token(tokens, i, COLON);
      i += 1;

      expr *a_cur_e = alloc(sizeof(expr));
      i = parse_expr(tokens, i, a_cur_e);
      vector_append(a_exprs, a_cur_e);

      if (peek_token(tokens, i) != COMMA)
        break;
      i += 1;
    }
    expect_token(tokens, i, RSQUARE);
    i += 1;

    aloop->vars_size = a_vars->size;
    if (a_vars->size == 0)
      aloop->vars = NULL;
    else
      aloop->vars = (char **)a_vars->data;

    aloop->list = alloc(sizeof(expr_list));
    aloop->list->exprs_size = a_exprs->size;
    aloop->list->exprs = (expr **)a_exprs->data;

    aloop->expr = alloc(sizeof(expr));
    i = parse_expr(tokens, i, aloop->expr);

    e->type = ARRAYLOOPEXPR;
    e->node = aloop;
    break;
  case SUM:
    if (e->node != NULL)
      parse_error(vector_get_token(tokens, i));
    sum_loop_expr *sloop = alloc(sizeof(sum_loop_expr));
    sloop->start = i;
    expect_token(tokens, i += 1, LSQUARE);
    i += 1;

    vector *s_vars = alloc(sizeof(vector));
    vector *s_exprs = alloc(sizeof(vector));
    vector_init(s_vars, 8, STRVECTOR);
    vector_init(s_exprs, 8, EXPRVECTOR);
    while (i < tokens->size) {
      if (peek_token(tokens, i) == RSQUARE)
        break;
      expect_token(tokens, i, VARIABLE);
      char *s_cur_str = vector_get_token(tokens, i)->text;
      char *s_cur_var = alloc(strlen(s_cur_str) + 1);
      memcpy(s_cur_var, s_cur_str, strlen(s_cur_str));
      vector_append(s_vars, s_cur_var);
      i += 1;

      expect_token(tokens, i, COLON);
      i += 1;

      expr *s_cur_e = alloc(sizeof(expr));
      i = parse_expr(tokens, i, s_cur_e);
      vector_append(s_exprs, s_cur_e);

      if (peek_token(tokens, i) != COMMA)
        break;
      i += 1;
    }
    expect_token(tokens, i, RSQUARE);
    i += 1;

    sloop->vars_size = s_vars->size;
    if (s_vars->size == 0)
      sloop->vars = NULL;
    else
      sloop->vars = (char **)s_vars->data;

    sloop->list = alloc(sizeof(expr_list));
    sloop->list->exprs_size = s_exprs->size;
    sloop->list->exprs = (expr **)s_exprs->data;

    sloop->expr = alloc(sizeof(expr));
    i = parse_expr(tokens, i, sloop->expr);

    e->type = SUMLOOPEXPR;
    e->node = sloop;
    break;
  // We should check unary here as our recursion does not allow for it
  // and its highest precedence so we can check first
  case OP:;
    char op = vector_get_token(tokens, i)->text[0];
    if (op != '!' && op != '-')
      break;
    if (e->node != NULL)
      break;
    unop_expr *unop = alloc(sizeof(unop_expr));
    unop->start = i;
    if (op == '!') {
      unop->op = NOTOP;
    } else if (op == '-') {
      unop->op = NEGOP;
    } else {
      free(unop);
      break;
    }
    i += 1;

    // Since base checks for unary and index
    // this should eval just fine
    unop->rhs = alloc(sizeof(expr));
    i = parse_base_level(tokens, i, unop->rhs);
    if (unop->rhs->node == NULL)
      parse_error(vector_get_token(tokens, i));

    e->type = UNOPEXPR;
    e->node = unop;
    break;
  default:
    break;
  }

  // We have already checked for unary so we can check for index
  // This way we only have to worry about binary ops in recursion
  i = parse_index_level(tokens, i, e);

  return i;
}

int parse_index_level(vector *tokens, int i, expr *e) {
  // If DOT or LSQUARE we have index
  int type = peek_token(tokens, i);
  // dot_expr
  if (type == DOT) {
    dot_expr *de = alloc(sizeof(dot_expr));
    de->start = e->start;
    i += 1;

    expect_token(tokens, i, VARIABLE);
    char *de_str = vector_get_token(tokens, i)->text;
    de->var = alloc(strlen(de_str) + 1);
    memcpy(de->var, de_str, strlen(de_str));

    expr *old_e = alloc(sizeof(expr));
    old_e->start = e->start;
    old_e->type = e->type;
    old_e->node = e->node;
    de->expr = old_e;

    e->type = DOTEXPR;
    e->node = de;
    i += 1;
  }
  // array_index_expr
  else if (type == LSQUARE) {
    array_index_expr *aie = alloc(sizeof(array_index_expr));
    aie->start = aie->start;
    aie->expr = e->node;
    i += 1;

    aie->list = NULL;
    if (peek_token(tokens, i) != RSQUARE) {
      aie->list = alloc(sizeof(expr_list));
      i = parse_expr_list(tokens, i, aie->list);
    }
    expect_token(tokens, i, RSQUARE);

    expr *old_e = alloc(sizeof(expr));
    old_e->start = e->start;
    old_e->type = e->type;
    old_e->node = e->node;
    aie->expr = old_e;

    e->type = ARRAYINDEXEXPR;
    e->node = aie;
    i += 1;
  } else {
    return i;
  }

  i = parse_index_level(tokens, i, e);
  return i;
}

int parse_mult_level(vector *tokens, int i, expr *e) {
  int type = peek_token(tokens, i);
  // Not mult, move deeper
  if (type != OP || e->node == NULL) {
    int old_i = i;
    i = parse_base_level(tokens, i, e);
    if (old_i == i)
      return i;
  } else {
    token *t = vector_get_token(tokens, i);
    char c = t->text[0];
    if (c == '*' || c == '%' || c == '/') {
      binop_expr *be = alloc(sizeof(binop_expr));
      be->start = i;
      i += 1;

      expr *old_e = alloc(sizeof(expr));
      old_e->start = e->start;
      old_e->type = e->type;
      old_e->node = e->node;
      be->lhs = old_e;

      be->rhs = alloc(sizeof(expr));
      i = parse_base_level(tokens, i, be->rhs);
      if (be->rhs->node == NULL)
        parse_error(vector_get_token(tokens, i));

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
      i = parse_base_level(tokens, i, e);
      if (old_i == i)
        return i;
    }
  }

  // Recurse at this level
  i = parse_mult_level(tokens, i, e);
  return i;
}

int parse_add_level(vector *tokens, int i, expr *e) {
  int type = peek_token(tokens, i);
  // Not add, move deeper
  if (type != OP || e->node == NULL) {
    int old_i = i;
    i = parse_mult_level(tokens, i, e);
    if (old_i == i)
      return i;
  } else {
    token *t = vector_get_token(tokens, i);
    char c = t->text[0];
    if (c == '+' || c == '-') {
      binop_expr *be = alloc(sizeof(binop_expr));
      be->start = i;
      i += 1;

      expr *old_e = alloc(sizeof(expr));
      old_e->start = e->start;
      old_e->type = e->type;
      old_e->node = e->node;
      be->lhs = old_e;

      be->rhs = alloc(sizeof(expr));
      i = parse_mult_level(tokens, i, be->rhs);
      if (be->rhs->node == NULL)
        parse_error(vector_get_token(tokens, i));

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
    // Not add, we need to go deeper
    else {
      int old_i = i;
      i = parse_mult_level(tokens, i, e);
      if (old_i == i)
        return i;
    }
  }

  // Recurse at this level
  i = parse_add_level(tokens, i, e);
  return i;
}

int parse_cmp_level(vector *tokens, int i, expr *e) {
  int type = peek_token(tokens, i);
  // Not cmp, move deeper
  if (type != OP || e->node == NULL) {
    int old_i = i;
    i = parse_add_level(tokens, i, e);
    if (old_i == i)
      return i;
  } else {
    token *t = vector_get_token(tokens, i);
    char c = t->text[0];
    if (c == '<' || c == '>' || c == '=' || c == '!') {
      if (c == '!' && strlen(t->text) == 1) {
        int old_i = i;
        i = parse_add_level(tokens, i, e);
        if (old_i == i)
          return i;
      } else {
        binop_expr *be = alloc(sizeof(binop_expr));
        be->start = i;
        i += 1;

        expr *old_e = alloc(sizeof(expr));
        old_e->start = e->start;
        old_e->type = e->type;
        old_e->node = e->node;
        be->lhs = old_e;

        be->rhs = alloc(sizeof(expr));
        i = parse_add_level(tokens, i, be->rhs);
        if (be->rhs->node == NULL)
          parse_error(vector_get_token(tokens, i));

        switch (c) {
        case '<':
          if (strlen(t->text) == 1)
            be->op = LTOP;
          else if (t->text[1] == '=')
            be->op = LEOP;
          else {
            parse_error(vector_get_token(tokens, i));
          }
          break;
        case '>':
          if (strlen(t->text) == 1)
            be->op = GTOP;
          else if (t->text[1] == '=')
            be->op = GEOP;
          else {
            parse_error(vector_get_token(tokens, i));
          }
          break;
        case '=':
          if (strlen(t->text) > 1 && t->text[1] == '=')
            be->op = EQOP;
          else {
            parse_error(vector_get_token(tokens, i));
          }
          break;
        case '!':
          if (strlen(t->text) > 1 && t->text[1] == '=')
            be->op = NEOP;
          else {
            parse_error(vector_get_token(tokens, i));
          }
          break;
        }

        e->type = BINOPEXPR;
        e->start = be->start;
        e->node = be;
      }
    }
    // Not cmp, we need to go deeper
    else {
      int old_i = i;
      i = parse_add_level(tokens, i, e);
      if (old_i == i)
        return i;
    }
  }

  // Recurse at this level
  i = parse_cmp_level(tokens, i, e);
  return i;
}

int parse_bool_level(vector *tokens, int i, expr *e) {
  int type = peek_token(tokens, i);
  // Not bool, move deeper
  if (type != OP || e->node == NULL) {
    int old_i = i;
    i = parse_cmp_level(tokens, i, e);
    if (old_i == i)
      return i;
  } else {
    token *t = vector_get_token(tokens, i);
    char c = t->text[0];
    if (c == '&' || c == '|') {
      binop_expr *be = alloc(sizeof(binop_expr));
      be->start = i;
      i += 1;

      expr *old_e = alloc(sizeof(expr));
      old_e->start = e->start;
      old_e->type = e->type;
      old_e->node = e->node;
      be->lhs = old_e;

      be->rhs = alloc(sizeof(expr));
      i = parse_cmp_level(tokens, i, be->rhs);
      if (be->rhs->node == NULL)
        parse_error(vector_get_token(tokens, i));

      switch (c) {
      case '&':
        if (strlen(t->text) > 1 && t->text[1] == '&')
          be->op = ANDOP;
        else {
          parse_error(vector_get_token(tokens, i));
        }
        break;
      case '|':
        if (strlen(t->text) > 1 && t->text[1] == '|')
          be->op = OROP;
        else {
          parse_error(vector_get_token(tokens, i));
        }
        break;
      }

      e->type = BINOPEXPR;
      e->start = be->start;
      e->node = be;
    }
    // Not bool, this is not a continuation
    else {
      int old_i = i;
      i = parse_cmp_level(tokens, i, e);
      if (old_i == i)
        return i;
    }
  }

  // Recurse at this level
  i = parse_bool_level(tokens, i, e);
  return i;
}

int parse_expr_list(vector *tokens, int i, expr_list *list) {
  vector *nodes = alloc(sizeof(vector));
  vector_init(nodes, 8, EXPRVECTOR);

  while (i < tokens->size - 1) {
    expr *e = alloc(sizeof(expr));
    e->start = i;
    i = parse_expr(tokens, i, e);
    vector_append(nodes, e);

    if (peek_token(tokens, i) != COMMA) {
      list->exprs_size = nodes->size;
      list->exprs = (expr **)nodes->data;
      break;
    }

    i += 1;
  }

  return i;
}
