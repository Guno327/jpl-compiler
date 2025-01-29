#include "parse_cmd.h"
#include "alloc.h"
#include "ast.h"
#include "compiler_error.h"
#include "parse_expr.h"
#include "parse_lvalue.h"
#include "parser.h"
#include <stdio.h>
#include <string.h>

int parse_cmd(TokenVector *tokens, int i, Cmd *c) {
  c->start = i;
  int type = peek_token(tokens, i);

  // Build sub-node
  // General form: Setup \n\n [single token step \n\n ...]
  switch (type) {
  case READ:;
    ReadCmd *rc = alloc(sizeof(ReadCmd));
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
    rc->lval = alloc(sizeof(VarLValue));
    i = parse_lvalue(tokens, i, rc->lval);
    c->node = rc;
    c->type = READCMD;
    break;
  case WRITE:;
    WriteCmd *wc = alloc(sizeof(WriteCmd));
    wc->start = i;
    i += 1;

    expect_token(tokens, i, IMAGE);
    i += 1;
    wc->expr = alloc(sizeof(Expr));
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
    LetCmd *lc = alloc(sizeof(LetCmd));
    lc->start = i;
    i += 1;

    lc->lval = alloc(sizeof(VarLValue));
    i = parse_lvalue(tokens, i, lc->lval);

    expect_token(tokens, i, EQUALS);
    i += 1;

    lc->expr = alloc(sizeof(Expr));
    i = parse_expr(tokens, i, lc->expr);
    c->node = lc;
    c->type = LETCMD;
    break;
  case ASSERT:;
    AssertCmd *ac = alloc(sizeof(AssertCmd));
    ac->start = i;
    i += 1;

    ac->expr = alloc(sizeof(Expr));
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
    PrintCmd *pc = alloc(sizeof(PrintCmd));
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
    ShowCmd *sc = alloc(sizeof(ShowCmd));
    sc->start = i;
    i += 1;

    sc->expr = alloc(sizeof(Expr));
    i = parse_expr(tokens, i, sc->expr);
    c->node = sc;
    c->type = SHOWCMD;
    break;
  case TIME:;
    TimeCmd *tc = alloc(sizeof(TimeCmd));
    tc->start = i;
    i += 1;

    tc->cmd = alloc(sizeof(Cmd));
    i = parse_cmd(tokens, i, tc->cmd);
    c->node = tc;
    c->type = TIMECMD;
    break;
  default:;
    char *msg = alloc(BUFSIZ);
    sprintf(msg, "Unexpected token '%s' at %d",
            vector_get_token(tokens, i)->text, i);
    parse_error(msg);
  }

  return i;
}
