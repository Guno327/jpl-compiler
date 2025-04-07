#include "parser.h"
#include "safe.h"
#include "ast.h"
#include "compiler_error.h"
#include "parse_cmd.h"
#include "token.h"
#include "vector_get.h"
#include <stdio.h>

vector *parse(vector *tokens) {
  vector *program = safe_alloc(sizeof(vector));
  vector_init(program, BUFSIZ, CMDVECTOR);

  long i = 0;
  for (; i < tokens->size; i++) {
    if (peek_token(tokens, i) == NEWLINE)
      continue;
    if (peek_token(tokens, i) == END_OF_FILE)
      break;

    cmd *c = safe_alloc(sizeof(cmd));
    i = parse_cmd(tokens, i, c);
    vector_append(program, c);
    expect_token(tokens, i, NEWLINE);
  }
  return program;
}

long peek_token(vector *tokens, long idx) {
  token *t = vector_get_token(tokens, idx);
  return t->type;
}

void expect_token(vector *tokens, long idx, long tok_type) {
  token *t = vector_get_token(tokens, idx);
  if (t->type != tok_type) {
    parse_error(vector_get_token(tokens, idx));
  }
}
