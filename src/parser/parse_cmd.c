#include "parse_cmd.h"
#include "ast.h"
#include "compiler_error.h"
#include "parse_expr.h"
#include "parse_lvalue.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parse_cmd(TokenVector *tokens, int i, Cmd *c) {
  c->start = i;
  int type = peek_token(tokens, i);

  // Build sub-node
  // General form: Setup \n\n [single token step \n\n ...]
  switch (type) {
  case READ:;
    ReadCmd *rc = malloc(sizeof(ReadCmd));
    memset(rc, 0, sizeof(ReadCmd));
    rc->start = i;
    i += 1;

    expect_token(tokens, i, IMAGE);
    expect_token(tokens, i + 1, STRING);
    i += 1;
    char *rc_str = vector_get_token(tokens, i)->text;
    rc->str = malloc(strlen(rc_str) + 1);
    memset(rc->str, 0, strlen(rc_str) + 1);
    memcpy(rc->str, rc_str, strlen(rc_str));
    i += 1;

    expect_token(tokens, i, TO);
    expect_token(tokens, i + 1, VARIABLE);
    i += 1;
    rc->lval = malloc(sizeof(VarLValue));
    memset(rc->lval, 0, sizeof(VarLValue));
    i = parse_lvalue(tokens, i, rc->lval);
    c->node = rc;
    c->type = READCMD;
    break;
  case WRITE:;
    WriteCmd *wc = malloc(sizeof(WriteCmd));
    memset(wc, 0, sizeof(WriteCmd));
    wc->start = i;
    i += 1;

    expect_token(tokens, i, IMAGE);
    i += 1;
    wc->expr = malloc(sizeof(Expr));
    memset(wc->expr, 0, sizeof(Expr));
    i = parse_expr(tokens, i, wc->expr);

    expect_token(tokens, i, TO);
    i += 1;
    expect_token(tokens, i, STRING);
    char *wc_str = vector_get_token(tokens, i)->text;
    wc->str = malloc(strlen(wc_str) + 1);
    memset(wc->str, 0, strlen(wc_str) + 1);
    memcpy(wc->str, wc_str, strlen(wc_str));
    i += 1;
    c->node = wc;
    c->type = WRITECMD;
    break;
  case LET:;
    LetCmd *lc = malloc(sizeof(LetCmd));
    memset(lc, 0, sizeof(LetCmd));
    lc->start = i;
    i += 1;

    lc->lval = malloc(sizeof(VarLValue));
    memset(lc->lval, 0, sizeof(VarLValue));
    i = parse_lvalue(tokens, i, lc->lval);

    expect_token(tokens, i, EQUALS);
    i += 1;

    lc->expr = malloc(sizeof(Expr));
    memset(lc->expr, 0, sizeof(Expr));
    i = parse_expr(tokens, i, lc->expr);
    c->node = lc;
    c->type = LETCMD;
    break;
  case ASSERT:;
    AssertCmd *ac = malloc(sizeof(AssertCmd));
    memset(ac, 0, sizeof(AssertCmd));
    ac->start = i;
    i += 1;

    ac->expr = malloc(sizeof(Expr));
    memset(ac->expr, 0, sizeof(Expr));
    i = parse_expr(tokens, i, ac->expr);

    expect_token(tokens, i, COMMA);
    expect_token(tokens, i + 1, STRING);
    i += 1;
    char *ac_str = vector_get_token(tokens, i)->text;
    ac->str = malloc(strlen(ac_str) + 1);
    memset(ac->str, 0, strlen(ac_str) + 1);
    memcpy(ac->str, ac_str, strlen(ac_str));
    i += 1;
    c->node = ac;
    c->type = ASSERTCMD;
    break;
  case PRINT:;
    PrintCmd *pc = malloc(sizeof(PrintCmd));
    memset(pc, 0, sizeof(PrintCmd));
    pc->start = i;
    i += 1;

    expect_token(tokens, i, STRING);
    char *pc_str = vector_get_token(tokens, i)->text;
    pc->str = malloc(strlen(pc_str) + 1);
    memset(pc->str, 0, strlen(pc_str) + 1);
    memcpy(pc->str, pc_str, strlen(pc_str));
    i += 1;
    c->node = pc;
    c->type = PRINTCMD;
    break;
  case SHOW:;
    ShowCmd *sc = malloc(sizeof(ShowCmd));
    memset(sc, 0, sizeof(ShowCmd));
    sc->start = i;
    i += 1;

    sc->expr = malloc(sizeof(Expr));
    memset(sc->expr, 0, sizeof(Expr));
    i = parse_expr(tokens, i, sc->expr);
    c->node = sc;
    c->type = SHOWCMD;
    break;
  case TIME:;
    TimeCmd *tc = malloc(sizeof(TimeCmd));
    memset(tc, 0, sizeof(TimeCmd));
    tc->start = i;
    i += 1;

    tc->cmd = malloc(sizeof(Cmd));
    memset(tc->cmd, 0, sizeof(Cmd));
    i = parse_cmd(tokens, i, tc->cmd);
    c->node = tc;
    c->type = TIMECMD;
    break;
  default:;
    char *msg = malloc(BUFSIZ);
    sprintf(msg, "Unexpected token '%s' at %d",
            vector_get_token(tokens, i)->text, i);
    parse_error(msg);
  }

  return i;
}
