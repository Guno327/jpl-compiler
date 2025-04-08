#include "ast.h"
#include "token.h"
#include "vector.h"
#ifndef PARSE_TYPE_H
#define PARSE_TYPE_H

long parse_type(vector *tokens, long i, type *t);
long parse_type_arr(vector *tokens, long i, type *t);

#endif
