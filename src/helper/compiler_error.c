#include "compiler_error.h"
#include "ast.h"
#include "t.h"
#include <stdio.h>
#include <stdlib.h>

void lex_error(char c, int i) {
  if (c == '\n')
    printf("Compilation Failed [LEXER] (%d): unexpected char NEWLINE\n", i);
  else
    printf("Compilation Failed [LEXER] (%d): unexpected char %c\n", i, c);
  exit(EXIT_FAILURE);
}

void parse_error(token *t) {
  char *token = print_token(t);
  printf("Compilation Failed [PARSER] (%d): unexpected token of type %s\n",
         t->start, token);
  exit(EXIT_FAILURE);
}

void typecheck_error(char *msg, int i) {
  printf("Compilation failed [TYPECHECKER] (%d): %s\n", i, msg);
  exit(EXIT_FAILURE);
}
