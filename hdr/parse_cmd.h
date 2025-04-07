#include "ast.h"
#include "token.h"
#include "vector.h"
#include <stdlib.h>
#ifndef PARSE_CMD_H
#define PARSE_CMD_H

long parse_cmd(vector *tokens, long i, cmd *c);
long parse_bindings(vector *tokens, long i, fn_cmd *fc);

#endif
