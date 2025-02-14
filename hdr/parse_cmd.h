#include "ast.h"
#include "token.h"
#include "vector.h"
#include <stdlib.h>
#ifndef PARSE_CMD_H
#define PARSE_CMD_H

int parse_cmd(vector *tokens, int i, cmd *c);
int parse_bindings(vector *tokens, int i, fn_cmd *fc);

#endif
