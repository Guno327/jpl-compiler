#include "parser.h"
#include "alloc.h"
#include "ast.h"
#include "compiler_error.h"
#include "parse_cmd.h"
#include "token.h"
#include <stdio.h>

Vector *parse(Vector *tokens) {
  Vector *program = alloc(sizeof(Vector));
  vector_init(program, BUFSIZ, CMDVECTOR);

  int i = 0;
  for (; i < tokens->size; i++) {
    if (peek_token(tokens, i) == NEWLINE)
      continue;
    if (peek_token(tokens, i) == END_OF_FILE)
      break;

    Cmd *c = alloc(sizeof(Cmd));
    i = parse_cmd(tokens, i, c);
    vector_append(program, c);
    expect_token(tokens, i, NEWLINE);
  }
  return program;
}

int peek_token(Vector *tokens, int idx) {
  Token *t = vector_get_token(tokens, idx);
  return t->type;
}

void expect_token(Vector *tokens, int idx, int tok_type) {
  Token *t = vector_get_token(tokens, idx);
  if (t->type != tok_type) {
    char *msg = alloc(BUFSIZ);
    sprintf(msg, "Unexpected token '%s' at %d",
            vector_get_token(tokens, idx)->text, idx);
    parse_error(msg);
  }
}
