#include "t.h"
#include "token.h"
#ifndef COMPILER_ERROR_H
#define COMPILER_ERROR_H

void lex_error(char c, int i);
void parse_error(token *t);
void typecheck_error(t *got, t *expected, int i);

#endif
