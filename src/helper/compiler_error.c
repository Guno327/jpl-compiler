#include "compiler_error.h"
#include "ast.h"
#include "t.h"
#include <stdio.h>
#include <stdlib.h>

void lex_error(char c, int i) {
  if (c == '\n')
    printf("Compilation Failed [LEXER]: unexpected char NEWLINE at %d\n", i);
  else
    printf("Compilation Failed [LEXER]: unexpected char %c at %d\n", c, i);
  exit(EXIT_FAILURE);
}

void parse_error(token *t) {
  char *token = print_token(t);
  printf("Compilation Failed [PARSER]: unexpected token of type %s starting at "
         "%d\n",
         token, t->start);
  exit(EXIT_FAILURE);
}

void typecheck_error(t *got, t *expected, int i) {
  char *got_str = t_to_str(got);
  char *expected_str = t_to_str(expected);
  printf("Compilation failed [TYPECHECKER]: expected type %s, got type %s, at "
         "%d\n",
         expected_str, got_str, i);
  exit(EXIT_FAILURE);
}
