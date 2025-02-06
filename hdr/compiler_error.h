#ifndef COMPILER_ERROR_H
#define COMPILER_ERROR_H
#include "token.h"

void lex_error(char c, int i);
void parse_error(Token *t);

#endif
