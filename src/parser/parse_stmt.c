#include "parse_stmt.h"
#include "alloc.h"
#include "compiler_error.h"
#include "parse_expr.h"
#include "parse_lvalue.h"
#include "parser.h"
#include <string.h>

int parse_stmt(Vector *tokens, int i, Stmt *s) {
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
    i = parse_expr(tokens, i, as->expr);

    expect_token(tokens, i, COMMA);
    i += 1;

    expect_token(tokens, i, STRING);
    char *as_str = vector_get_token(tokens, i)->text;
    as->str = alloc(strlen(as_str) + 1);
    memcpy(as->str, as_str, strlen(as_str));
    i += 1;

    s->type = ASSERTSTMT;
    s->node = as;
    break;
  case RETURN:;
    ReturnStmt *rs = alloc(sizeof(ReturnStmt));
    rs->start = i;
    i += 1;

    rs->expr = alloc(sizeof(Expr));
    i = parse_expr(tokens, i, rs->expr);

    s->type = RETURNSTMT;
    s->node = rs;
    break;
  default:;
    parse_error(vector_get_token(tokens, i));
  }

  return i;
}
