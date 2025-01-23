#include "error.h"
#include <stdio.h>
#include <stdlib.h>

void lex_error(char *msg) {
  fprintf(stderr, "LEXER:%s\n", msg);
  printf("Compilation failed\n");
  exit(EXIT_FAILURE);
}

void parse_error(char *msg) {
  fprintf(stderr, "PARSER:%s\n", msg);
  printf("Compilation failed\n");
  exit(EXIT_FAILURE);
}
