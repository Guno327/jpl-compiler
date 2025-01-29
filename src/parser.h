#ifndef PARSER_H
#define PARSER_H
#include "ast.h"
#include "token.h"
#include "vector.h"

CmdVector *parse(TokenVector *tokens);
int peek_token(TokenVector *tokens, int idx);
void expect_token(TokenVector *tokens, int idx, int tok_type);

#endif
