#include "parse_type.h"
#include "safe.h"
#include "ast.h"
#include "compiler_error.h"
#include "parser.h"
#include "vector_get.h"
#include <string.h>

int parse_type(vector *tokens, int i, type *t) {
  t->start = vector_get_token(tokens, i)->start;
  switch (peek_token(tokens, i)) {
  case INT:;
    int_type *it = safe_alloc(sizeof(int_type));
    it->start = vector_get_token(tokens, i)->start;
    t->type = INTTYPE;
    t->node = it;
    i += 1;
    break;
  case FLOAT:;
    float_type *ft = safe_alloc(sizeof(float_type));
    ft->start = vector_get_token(tokens, i)->start;
    t->type = FLOATTYPE;
    t->node = ft;
    i += 1;
    break;
  case BOOL:;
    bool_type *bt = safe_alloc(sizeof(bool_type));
    bt->start = vector_get_token(tokens, i)->start;
    t->type = BOOLTYPE;
    t->node = bt;
    i += 1;
    break;
  case VARIABLE:;
    struct_type *st = safe_alloc(sizeof(struct_type));
    memset(st, 0, sizeof(struct_type));
    st->start = vector_get_token(tokens, i)->start;

    char *st_str = vector_get_token(tokens, i)->text;
    char *st_var = safe_alloc(strlen(st_str) + 1);
    memcpy(st_var, st_str, strlen(st_str));
    st->var = st_var;

    t->type = STRUCTTYPE;
    t->node = st;
    i += 1;
    break;
  case VOID:;
    void_type *vt = safe_alloc(sizeof(void_type));
    vt->start = vector_get_token(tokens, i)->start;
    t->type = VOIDTYPE;
    t->node = vt;
    i += 1;
    break;
  default:;
    parse_error(vector_get_token(tokens, i));
  }

  i = parse_type_arr(tokens, i, t);

  return i;
}

int parse_type_arr(vector *tokens, int i, type *t) {
  int token_t = peek_token(tokens, i);
  if (token_t != LSQUARE)
    return i;

  array_type *at = safe_alloc(sizeof(array_type));
  at->start = t->start;
  at->type = t->node;
  i += 1;

  token_t = peek_token(tokens, i);
  at->rank = 1;
  if (token_t != RSQUARE) {
    expect_token(tokens, i, COMMA);
    while (token_t == COMMA) {
      at->rank += 1;
      i += 1;
      token_t = peek_token(tokens, i);
    }

    expect_token(tokens, i, RSQUARE);
  }

  type *old_t = safe_alloc(sizeof(type));
  old_t->type = t->type;
  old_t->start = t->start;
  old_t->node = t->node;
  at->type = old_t;

  t->type = ARRAYTYPE;
  t->node = at;
  i += 1;
  i = parse_type_arr(tokens, i, t);
  return i;
}
