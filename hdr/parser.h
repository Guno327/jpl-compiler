#include "ast.h"
#include "token.h"
#include "vector.h"
#ifndef PARSER_H
#define PARSER_H

vector *parse(vector *tokens);
long peek_token(vector *tokens, long idx);
void expect_token(vector *tokens, long idx, long tok_type);

#endif
