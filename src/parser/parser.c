#include "parser.h"
#include "alloc.h"
#include "ast.h"
#include "compiler_error.h"
#include "parse_cmd.h"
#include "token.h"
#include "vector_get.h"
#include <stdio.h>

vector *parse(vector *tokens) {
  vector *program = alloc(sizeof(vector));
  vector_init(program, BUFSIZ, CMDVECTOR);

  int i = 0;
  for (; i < tokens->size; i++) {
    if (peek_token(tokens, i) == NEWLINE)
      continue;
    if (peek_token(tokens, i) == END_OF_FILE)
      break;

    cmd *c = alloc(sizeof(cmd));
    i = parse_cmd(tokens, i, c);
    vector_append(program, c);
    expect_token(tokens, i, NEWLINE);
  }
  return program;
}

int peek_token(vector *tokens, int idx) {
  token *t = vector_get_token(tokens, idx);
  return t->type;
}

void expect_token(vector *tokens, int idx, int tok_type) {
  token *t = vector_get_token(tokens, idx);
  if (t->type != tok_type) {
    parse_error(vector_get_token(tokens, idx));
  }
}
