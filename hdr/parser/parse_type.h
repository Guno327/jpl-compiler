#ifndef PARSE_TYPE_H
#define PARSE_TYPE_H
#include "token.h"
#include "vector.h"

int parse_type(TokenVector *tokens, int i, Type *t);
int parse_type_arr(TokenVector *tokens, int i, TypeArr *ta);

#endif
