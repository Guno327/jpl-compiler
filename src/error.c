#include <stdio.h>
#include <stdlib.h>
#include "error.h"

void lex_error(char* msg, char c, int i){
  printf("Compilation failed: ");
  fprintf(stderr, msg, c, i);
  printf("\n");
  exit(EXIT_FAILURE);
}
