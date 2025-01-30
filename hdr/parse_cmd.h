#ifndef PARSE_CMD_H
#define PARSE_CMD_H
#include "ast.h"
#include "token.h"
#include "vector.h"
#include <stdlib.h>

int parse_cmd(Vector *tokens, int i, Cmd *c);
int parse_bindings(Vector *tokens, int i, FnCmd *fc);

#endif
