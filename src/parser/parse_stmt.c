#include "parse_stmt.h"
#include "safe.h"
#include "compiler_error.h"
#include "parse_expr.h"
#include "parse_lval.h"
#include "parser.h"
#include "vector_get.h"
#include <string.h>

int parse_stmt(vector *tokens, int i, stmt *s) {
  s->start = vector_get_token(tokens, i)->start;

  switch (peek_token(tokens, i)) {
  case LET:;
    let_stmt *ls = safe_alloc(sizeof(let_stmt));
    ls->start = vector_get_token(tokens, i)->start;
    i += 1;

    ls->lval = safe_alloc(sizeof(lval));
    i = parse_lvalue(tokens, i, ls->lval);

    expect_token(tokens, i, EQUALS);
    i += 1;

    ls->expr = safe_alloc(sizeof(expr));
    i = parse_expr(tokens, i, ls->expr);

    s->type = LETSTMT;
    s->node = ls;
    break;
  case ASSERT:;
    assert_stmt *as = safe_alloc(sizeof(assert_stmt));
    as->start = vector_get_token(tokens, i)->start;
    i += 1;

    as->expr = safe_alloc(sizeof(expr));
    i = parse_expr(tokens, i, as->expr);

    expect_token(tokens, i, COMMA);
    i += 1;

    expect_token(tokens, i, STRING);
    char *as_str = vector_get_token(tokens, i)->text;
    as->str = safe_alloc(strlen(as_str) + 1);
    memcpy(as->str, as_str, strlen(as_str));
    i += 1;

    s->type = ASSERTSTMT;
    s->node = as;
    break;
  case RETURN:;
    return_stmt *rs = safe_alloc(sizeof(return_stmt));
    rs->start = vector_get_token(tokens, i)->start;
    i += 1;

    rs->expr = safe_alloc(sizeof(expr));
    i = parse_expr(tokens, i, rs->expr);

    s->type = RETURNSTMT;
    s->node = rs;
    break;
  default:;
    parse_error(vector_get_token(tokens, i));
  }

  return i;
}
