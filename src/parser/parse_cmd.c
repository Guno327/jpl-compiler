#include "parse_cmd.h"
#include "alloc.h"
#include "ast.h"
#include "compiler_error.h"
#include "parse_expr.h"
#include "parse_lval.h"
#include "parse_stmt.h"
#include "parse_type.h"
#include "parser.h"
#include "vector_get.h"
#include <stdio.h>
#include <string.h>

int parse_cmd(vector *tokens, int i, cmd *c) {
  c->start = i;
  int token_t = peek_token(tokens, i);

  // Build sub-node
  // General form: Setup \n\n [single token step \n\n ...]
  switch (token_t) {
  case READ:;
    read_cmd *rc = alloc(sizeof(read_cmd));
    rc->start = i;
    i += 1;

    expect_token(tokens, i, IMAGE);
    expect_token(tokens, i + 1, STRING);
    i += 1;
    char *rc_str = vector_get_token(tokens, i)->text;
    rc->str = alloc(strlen(rc_str) + 1);
    memcpy(rc->str, rc_str, strlen(rc_str));
    i += 1;

    expect_token(tokens, i, TO);
    expect_token(tokens, i + 1, VARIABLE);
    i += 1;
    rc->lval = alloc(sizeof(var_lval));
    i = parse_lvalue(tokens, i, rc->lval);
    c->node = rc;
    c->type = READCMD;
    break;
  case WRITE:;
    write_cmd *wc = alloc(sizeof(write_cmd));
    wc->start = i;
    i += 1;

    expect_token(tokens, i, IMAGE);
    i += 1;
    wc->expr = alloc(sizeof(expr));
    i = parse_expr(tokens, i, wc->expr);

    expect_token(tokens, i, TO);
    i += 1;
    expect_token(tokens, i, STRING);
    char *wc_str = vector_get_token(tokens, i)->text;
    wc->str = alloc(strlen(wc_str) + 1);
    memcpy(wc->str, wc_str, strlen(wc_str));
    i += 1;
    c->node = wc;
    c->type = WRITECMD;
    break;
  case LET:;
    let_cmd *lc = alloc(sizeof(let_cmd));
    lc->start = i;
    i += 1;

    lc->lval = alloc(sizeof(var_lval));
    i = parse_lvalue(tokens, i, lc->lval);

    expect_token(tokens, i, EQUALS);
    i += 1;

    lc->expr = alloc(sizeof(expr));
    i = parse_expr(tokens, i, lc->expr);
    c->node = lc;
    c->type = LETCMD;
    break;
  case ASSERT:;
    assert_cmd *ac = alloc(sizeof(assert_cmd));
    ac->start = i;
    i += 1;

    ac->expr = alloc(sizeof(expr));
    i = parse_expr(tokens, i, ac->expr);

    expect_token(tokens, i, COMMA);
    expect_token(tokens, i + 1, STRING);
    i += 1;
    char *ac_str = vector_get_token(tokens, i)->text;
    ac->str = alloc(strlen(ac_str) + 1);
    memcpy(ac->str, ac_str, strlen(ac_str));
    i += 1;
    c->node = ac;
    c->type = ASSERTCMD;
    break;
  case PRINT:;
    print_cmd *pc = alloc(sizeof(print_cmd));
    pc->start = i;
    i += 1;

    expect_token(tokens, i, STRING);
    char *pc_str = vector_get_token(tokens, i)->text;
    pc->str = alloc(strlen(pc_str) + 1);
    memcpy(pc->str, pc_str, strlen(pc_str));
    i += 1;
    c->node = pc;
    c->type = PRINTCMD;
    break;
  case SHOW:;
    show_cmd *sc = alloc(sizeof(show_cmd));
    sc->start = i;
    i += 1;

    sc->expr = alloc(sizeof(expr));
    i = parse_expr(tokens, i, sc->expr);
    c->node = sc;
    c->type = SHOWCMD;
    break;
  case TIME:;
    time_cmd *tc = alloc(sizeof(time_cmd));
    tc->start = i;
    i += 1;

    tc->cmd = alloc(sizeof(cmd));
    i = parse_cmd(tokens, i, tc->cmd);
    c->node = tc;
    c->type = TIMECMD;
    break;
  case FN:;
    fn_cmd *fc = alloc(sizeof(fn_cmd));
    fc->start = i;
    i += 1;

    expect_token(tokens, i, VARIABLE);
    char *fc_str = vector_get_token(tokens, i)->text;
    fc->var = alloc(strlen(fc_str) + 1);
    memcpy(fc->var, fc_str, strlen(fc_str));
    i += 1;

    expect_token(tokens, i, LPAREN);
    i += 1;

    fc->binds = NULL;
    if (peek_token(tokens, i) != RPAREN)
      i = parse_bindings(tokens, i, fc);
    expect_token(tokens, i, RPAREN);
    i += 1;

    expect_token(tokens, i, COLON);
    i += 1;

    fc->type = alloc(sizeof(type));
    i = parse_type(tokens, i, fc->type);

    expect_token(tokens, i, LCURLY);
    expect_token(tokens, i + 1, NEWLINE);
    i += 2;

    fc->stmts = NULL;
    vector *stmts = alloc(sizeof(vector));
    vector_init(stmts, 8, STMTVECTOR);
    while (peek_token(tokens, i) != RCURLY) {
      stmt *s = alloc(sizeof(stmt));
      i = parse_stmt(tokens, i, s);
      vector_append(stmts, s);

      expect_token(tokens, i, NEWLINE);
      i += 1;
      if (peek_token(tokens, i) == RCURLY)
        break;
      else
        continue;

      parse_error(vector_get_token(tokens, i));
    }
    expect_token(tokens, i, RCURLY);
    fc->stmts = stmts;

    c->type = FNCMD;
    c->node = fc;
    i += 1;
    break;
  case STRUCT:;
    struct_cmd *stc = alloc(sizeof(struct_cmd));
    stc->start = i;
    i += 1;

    expect_token(tokens, i, VARIABLE);
    char *stc_str = vector_get_token(tokens, i)->text;
    stc->var = alloc(strlen(stc_str) + 1);
    memcpy(stc->var, stc_str, strlen(stc_str));
    i += 1;

    expect_token(tokens, i, LCURLY);
    expect_token(tokens, i + 1, NEWLINE);
    i += 2;

    stc->types = alloc(sizeof(vector));
    stc->vars = alloc(sizeof(vector));
    vector_init(stc->types, 8, TYPEVECTOR);
    vector_init(stc->vars, 8, STRVECTOR);
    while (peek_token(tokens, i) != RCURLY) {
      expect_token(tokens, i, VARIABLE);
      char *str = vector_get_token(tokens, i)->text;
      char *var = alloc(strlen(str) + 1);
      memcpy(var, str, strlen(str));
      vector_append(stc->vars, var);
      i += 1;

      expect_token(tokens, i, COLON);
      i += 1;

      type *t = alloc(sizeof(type));
      i = parse_type(tokens, i, t);
      vector_append(stc->types, t);

      expect_token(tokens, i, NEWLINE);
      i += 1;
      if (peek_token(tokens, i) == RCURLY)
        break;
      else
        continue;

      parse_error(vector_get_token(tokens, i));
    }
    expect_token(tokens, i, RCURLY);

    c->type = STRUCTCMD;
    c->node = stc;
    i += 1;
    break;
  default:;
    parse_error(vector_get_token(tokens, i));
  }

  return i;
}

int parse_bindings(vector *tokens, int i, fn_cmd *fc) {
  vector *binds = alloc(sizeof(vector));
  vector_init(binds, 8, BINDINGVECTOR);

  while (i < tokens->size) {
    binding *cur_b = alloc(sizeof(binding));
    cur_b->lval = alloc(sizeof(lval));
    cur_b->type = alloc(sizeof(type));
    i = parse_lvalue(tokens, i, cur_b->lval);
    expect_token(tokens, i, COLON);
    i += 1;
    i = parse_type(tokens, i, cur_b->type);
    vector_append(binds, cur_b);

    if (peek_token(tokens, i) != COMMA)
      break;

    i += 1;
  }

  fc->binds = binds;

  return i;
}
