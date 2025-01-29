#include "parse_stmt.h"
#include "alloc.h"
#include "parse_expr.h"
#include "parse_lvalue.h"
#include "parser.h"

int parse_stmt(TokenVector *tokens, int i, Stmt *s) {
  s->start = i;

  switch (peek_token(tokens, i)) {
  case LET:;
    LetStmt *ls = alloc(sizeof(LetStmt));
    ls->start = i;
    i += 1;

    ls->lval = alloc(sizeof(LValue));
    i = parse_lvalue(tokens, i, ls->lval);

    expect_token(tokens, i, EQUALS);
    i += 1;

    ls->expr = alloc(sizeof(Expr));
    i = parse_expr(tokens, i, ls->expr);

    s->type = LETSTMT;
    s->node = ls;
    break;
  case ASSERT:;
    AssertStmt *as = alloc(sizeof(AssertStmt));
    as->start = i;
    i += 1;

    as->expr = alloc(sizeof(Expr));
  case RETURN:
  default:
  }
}
