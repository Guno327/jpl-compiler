#include "parse_type.h"
#include "ast.h"
#include "error.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parse_type(TokenVector *tokens, int i, Type *t) {
  t->start = i;
  switch (peek_token(tokens, i)) {
  case INT:;
    IntType *it = malloc(sizeof(IntType));
    memset(it, 0, sizeof(IntType));
    it->start = i;
    t->type = INTTYPE;
    t->node = it;
    i += 1;
    break;
  case FLOAT:;
    FloatType *ft = malloc(sizeof(FloatType));
    memset(ft, 0, sizeof(FloatType));
    ft->start = i;
    t->type = FLOATTYPE;
    t->node = ft;
    i += 1;
    break;
  case BOOL:;
    BoolType *bt = malloc(sizeof(BoolType));
    memset(bt, 0, sizeof(BoolType));
    bt->start = i;
    t->type = BOOLTYPE;
    t->node = bt;
    i += 1;
    break;
  case VARIABLE:;
    StructType *st = malloc(sizeof(StructType));
    memset(st, 0, sizeof(StructType));
    st->start = i;

    char *st_str = vector_get_token(tokens, i)->text;
    char *st_var = malloc(strlen(st_str) + 1);
    memset(st_var, 0, strlen(st_str) + 1);
    memcpy(st_var, st_str, strlen(st_str));
    st->var = st_var;

    t->type = STRUCTTYPE;
    t->node = st;
    i += 1;
    break;
  case VOID:;
    VoidType *vt = malloc(sizeof(VoidType));
    memset(vt, 0, sizeof(VoidType));
    vt->start = i;
    t->node = vt;
    i += 1;
    break;
  default:;
    char *msg = malloc(BUFSIZ);
    sprintf(msg, "Unexpected token '%s' at %d",
            vector_get_token(tokens, i)->text, i);
    parse_error(msg);
  }

  if (peek_token(tokens, i) == LSQUARE) {
    ArrayType *at = malloc(sizeof(ArrayType));
    memset(at, 0, sizeof(ArrayType));

    at->type = malloc(sizeof(Type));
    memset(at->type, 0, sizeof(Type));
    at->type->type = t->type;
    at->type->start = t->start;
    at->type->node = t->node;

    at->arr = malloc(sizeof(TypeArr));
    memset(at->arr, 0, sizeof(TypeArr));
    i = parse_type_arr(tokens, i, at->arr);

    t->type = ARRAYTYPE;
    t->node = at;
  }

  return i;
}

int parse_type_arr(TokenVector *tokens, int i, TypeArr *ta) {
  ta->start = i;
  expect_token(tokens, i, LSQUARE);
  i += 1;

  ta->rank = 1;
  for (; i < tokens->size; i++) {
    if (peek_token(tokens, i) == RSQUARE) {
      i += 1;
      break;
    }

    if (peek_token(tokens, i) == COMMA) {
      ta->rank += 1;
      continue;
    }

    char *msg = malloc(BUFSIZ);
    sprintf(msg, "Unexpected token '%s' t %d",
            vector_get_token(tokens, i)->text, i);
    parse_error(msg);
  }

  if (peek_token(tokens, i) == LSQUARE) {
    ta->typearr = malloc(sizeof(TypeArr));
    memset(ta->typearr, 0, sizeof(TypeArr));
    i = parse_type_arr(tokens, i, ta->typearr);
  }

  return i;
}
