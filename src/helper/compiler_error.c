#include "compiler_error.h"
#include <stdio.h>
#include <stdlib.h>

void lex_error(char c, int i) {
  if (c == '\n')
    printf("Compilation Failed: unexpected char NEWLINE at %d\n", i);
  else
    printf("Compilation Failed: unexpected char %c at %d\n", c, i);
  exit(EXIT_FAILURE);
}

void parse_error(token *t) {
  char *token = print_token(t);
  printf("Compilation Failed: unexpected token of type %s starting at %d\n",
         token, t->start);
  exit(EXIT_FAILURE);
}
