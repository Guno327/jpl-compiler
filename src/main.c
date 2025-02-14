#include "alloc.h"
#include "lexer.h"
#include "parser.h"
#include "vector.h"
#include "vector_get.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { LEX, PARSE, TYPECHECK, ALL } RunMode;

int main(int argc, char **argv) {
  RunMode mode = LEX;

  // Handle args
  if (argc < 2) {
    fprintf(stderr, "No file given\n");
    exit(EXIT_FAILURE);
  };

  char *filename = NULL;
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-l"))
      mode = LEX;
    else if (!strcmp(argv[i], "-p"))
      mode = PARSE;
    else if (!strcmp(argv[i], "-t"))
      mode = TYPECHECK;
    else
      filename = argv[i];
  }

  // Open File
  FILE *src_file = NULL;
  src_file = fopen(filename, "r");
  if (src_file == NULL) {
    fprintf(stderr, "Could not open file: %s\n", argv[2]);
    exit(EXIT_FAILURE);
  };

  // Read File
  char *src = alloc(BUFSIZ);
  char *buf = alloc(BUFSIZ);

  int read = 0;
  char *ptr = NULL;

  do {
    ptr = fgets(buf, BUFSIZ, src_file);
    if (ptr == NULL) {
      if (feof(src_file))
        break;
      else {
        fprintf(stderr, "Compilation failed: Error reading file at %lu\n",
                strlen(src));
        exit(EXIT_FAILURE);
      }
    }
    read = strlen(ptr);
    src = realloc(src, strlen(src) + read + 1);

    if (src == NULL) {
      fprintf(stderr, "Malloc failed\n");
      exit(EXIT_FAILURE);
    }

    strncat(src, ptr, read);
    memset(ptr, 0, BUFSIZ);
    ptr = NULL;
  } while (read != 0);
  fclose(src_file);

  // Lex
  vector *tokens = lex(src);
  free(src);

  if (mode == LEX) {
    for (int i = 0; i < tokens->size; i++) {
      token *t = vector_get_token(tokens, i);
      char *t_str = print_token(t);
      printf("%s\n", t_str);
      free(t_str);
    }

    printf("Compilation succeeded: lexical analysis complete\n");
    exit(EXIT_SUCCESS);
  }

  // Parse
  vector *program = parse(tokens);
  free(tokens);

  // Print
  if (mode == PARSE) {
    for (int i = 0; i < program->size; i++) {
      printf("%s\n", cmd_to_str(vector_get_cmd(program, i)));
    }
    free(program);
    printf("Compilation succeeded: parsing complete\n");

    exit(EXIT_SUCCESS);
  }

  exit(EXIT_SUCCESS);
}
