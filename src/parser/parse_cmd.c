#include "parse_cmd.h"
#include "alloc.h"
#include "ast.h"
#include "compiler_error.h"
#include "parse_expr.h"
#include "parse_lvalue.h"
#include "parse_stmt.h"
#include "parse_type.h"
#include "parser.h"
#include <stdio.h>
#include <string.h>

int parse_cmd(Vector *tokens, int i, Cmd *c) {
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
  case FN:;
    FnCmd *fc = alloc(sizeof(FnCmd));
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
      i = parse_bindings(tokens, i, fc->binds, &fc->binds_size);
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
    Vector *stmts = alloc(sizeof(Vector));
    vector_init(stmts, 8, STMTVECTOR);
    while (peek_token(tokens, i) != RCURLY) {
      Stmt *s = alloc(sizeof(Stmt));
      i = parse_stmt(tokens, i, s);
      vector_append(stmts, s);

      if (peek_token(tokens, i) == NEWLINE) {
        i += 1;
        continue;
      }
      if (peek_token(tokens, i) == RCURLY)
        break;

      char *msg = alloc(BUFSIZ);
      sprintf(msg, "Unexpected token '%s' at %d",
              vector_get_token(tokens, i)->text, i);
      parse_error(msg);
    }
    expect_token(tokens, i, RCURLY);

    c->type = FNCMD;
    c->node = fc;
    i += 1;
    break;
  case STRUCT:;
    StructCmd *stc = alloc(sizeof(StructCmd));
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

    stc->types = NULL;
    stc->vars = NULL;
    Vector *types = alloc(sizeof(Vector));
    Vector *vars = alloc(sizeof(Vector));
    vector_init(types, 8, TYPEVECTOR);
    vector_init(vars, 8, STRVECTOR);
    while (peek_token(tokens, i) != RCURLY) {
      expect_token(tokens, i, VARIABLE);
      char *str = vector_get_token(tokens, i)->text;
      char *var = alloc(strlen(str) + 1);
      memcpy(var, str, strlen(str));
      vector_append(vars, var);
      i += 1;

      expect_token(tokens, i, COLON);
      i += 1;

      Type *t = alloc(sizeof(Type));
      i = parse_type(tokens, i, t);
      vector_append(types, t);

      if (peek_token(tokens, i) == NEWLINE) {
        i += 1;
        continue;
      }
      if (peek_token(tokens, i) == RCURLY)
        break;

      char *msg = alloc(BUFSIZ);
      sprintf(msg, "Unexpected token '%s' at %d",
              vector_get_token(tokens, i)->text, i);
      parse_error(msg);
    }
    expect_token(tokens, i, RCURLY);

    c->type = STRUCTCMD;
    c->node = stc;
    i += 1;
    break;
  default:;
    char *msg = alloc(BUFSIZ);
    sprintf(msg, "Unexpected token '%s' at %d",
            vector_get_token(tokens, i)->text, i);
    parse_error(msg);
  }

  return i;
}

int parse_bindings(Vector *tokens, int i, Binding **b, int *b_size) {
  Vector *lvals = alloc(sizeof(Vector));
  Vector *types = alloc(sizeof(Vector));
  vector_init(lvals, 8, LVALUEVECTOR);
  vector_init(types, 8, TYPEVECTOR);

  while (i < tokens->size) {
    LValue *lval = alloc(sizeof(LValue));
    i = parse_lvalue(tokens, i, lval);
    expect_token(tokens, i, COLON);
    i += 1;

    Type *type = alloc(sizeof(Type));
    i = parse_type(tokens, i, type);

    if (peek_token(tokens, i) != COMMA)
      break;

    i += 1;
  }

  *b_size = lvals->size;
  b = alloc(sizeof(Binding *) * *b_size);
  for (int j = 0; j < lvals->size; j++) {
    Binding *cur_b = alloc(sizeof(Binding));
    cur_b->lval = vector_get_lvalue(lvals, j);
    cur_b->type = vector_get_type(types, j);
    b[j] = cur_b;
  }

  return i;
}
