#include "parser.h"
#include "ast.h"
#include "compiler_error.h"
#include "parse_cmd.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CmdVector *parse(TokenVector *tokens) {
  CmdVector *program = malloc(sizeof(CmdVector));
  vector_init_cmd(program, BUFSIZ);

  int i = 0;
  for (; i < tokens->size; i++) {
    if (peek_token(tokens, i) == NEWLINE)
      continue;
    if (peek_token(tokens, i) == END_OF_FILE)
      break;

    Cmd *c = malloc(sizeof(Cmd));
    memset(c, 0, sizeof(Cmd));
    i = parse_cmd(tokens, i, c);
    vector_append_cmd(program, c);
    expect_token(tokens, i, NEWLINE);
  }
  return program;
}

int peek_token(TokenVector *tokens, int idx) {
  Token *t = vector_get_token(tokens, idx);
  return t->type;
}

void expect_token(TokenVector *tokens, int idx, int tok_type) {
  Token *t = vector_get_token(tokens, idx);
  if (t->type != tok_type) {
    char *msg = malloc(BUFSIZ);
    sprintf(msg, "Unexpected token '%s' at %d",
            vector_get_token(tokens, idx)->text, idx);
    parse_error(msg);
  }
}
