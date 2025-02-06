#include "parse_type.h"
#include "alloc.h"
#include "ast.h"
#include "compiler_error.h"
#include "parser.h"
#include <stdio.h>
#include <string.h>

int parse_type(Vector *tokens, int i, Type *t) {
  t->start = i;
  switch (peek_token(tokens, i)) {
  case INT:;
    IntType *it = alloc(sizeof(IntType));
    it->start = i;
    t->type = INTTYPE;
    t->node = it;
    i += 1;
    break;
  case FLOAT:;
    FloatType *ft = alloc(sizeof(FloatType));
    ft->start = i;
    t->type = FLOATTYPE;
    t->node = ft;
    i += 1;
    break;
  case BOOL:;
    BoolType *bt = alloc(sizeof(BoolType));
    bt->start = i;
    t->type = BOOLTYPE;
    t->node = bt;
    i += 1;
    break;
  case VARIABLE:;
    StructType *st = alloc(sizeof(StructType));
    memset(st, 0, sizeof(StructType));
    st->start = i;

    char *st_str = vector_get_token(tokens, i)->text;
    char *st_var = alloc(strlen(st_str) + 1);
    memcpy(st_var, st_str, strlen(st_str));
    st->var = st_var;

    t->type = STRUCTTYPE;
    t->node = st;
    i += 1;
    break;
  case VOID:;
    VoidType *vt = alloc(sizeof(VoidType));
    vt->start = i;
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

int parse_type_arr(Vector *tokens, int i, Type *t) {
  int type = peek_token(tokens, i);
  if (type != LSQUARE)
    return i;

  ArrayType *at = alloc(sizeof(ArrayType));
  at->start = t->start;
  at->type = t->node;
  i += 1;

  type = peek_token(tokens, i);
  at->rank = 1;
  if (type != RSQUARE) {
    expect_token(tokens, i, COMMA);
    while (type == COMMA) {
      at->rank += 1;
      i += 1;
      type = peek_token(tokens, i);
    }

    expect_token(tokens, i, RSQUARE);
  }

  Type *old_t = alloc(sizeof(Type));
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
