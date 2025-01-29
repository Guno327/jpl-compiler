#ifndef PARSE_CMD_H
#define PARSE_CMD_H
#include "token.h"
#include "vector.h"

int parse_cmd(TokenVector *tokens, int i, Cmd *c);
int parse_binding(TokenVector *tokens, int i, Type *t);

#endif
