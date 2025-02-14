#include "token.h"
#include "vector.h"
#ifndef PARSE_TYPE_H
#define PARSE_TYPE_H

int parse_type(vector *tokens, int i, type *t);
int parse_type_arr(vector *tokens, int i, type *t);

#endif
