#include "ast.h"
#include "token.h"
#include "vector.h"
#ifndef PARSER_H
#define PARSER_H

vector *parse(vector *tokens);
int peek_token(vector *tokens, int idx);
void expect_token(vector *tokens, int idx, int tok_type);

#endif
