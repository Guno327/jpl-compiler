#include "lexer.h"
#include "vector.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {LEX, PARSE, TYPECHECK, ALL} RunMode;

int main(int argc, char **argv) {
  RunMode mode = ALL;

  // Handle args
  if (argc < 2) {
    fprintf(stderr, "No file given\n");
    exit(EXIT_FAILURE);
  };

  char* filename = NULL;
  for(int i = 0; i < argc; i++){
    if(!strcmp(argv[i], "-l"))
      mode = LEX;
    else if (!strcmp(argv[i], "-p"))
        mode = PARSE;
    else if (!strcmp(argv[i], "-t"))
        mode = TYPECHECK;
    else if (strchr(argv[i], '-') == NULL)
        filename = argv[i];
    else{
        printf("Unknown flag: %s\n", argv[i]);
      exit(EXIT_SUCCESS);
    }
  }

  // Open File
  FILE *src_file = NULL;
  src_file = fopen(filename, "r");
  if (src_file == NULL) {
    fprintf(stderr, "Could not open file: %s\n", argv[2]);
    exit(EXIT_FAILURE);
  };

  // Read File
  char *src = malloc(BUFSIZ);
  char *buf = malloc(BUFSIZ);
  memset(src, 0, BUFSIZ);
  memset(buf, 0, BUFSIZ);

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
  Vector *tokens = lex(src);
  free(src);

  if (mode == LEX){
    for (int i = 0; i < tokens->size; i++) {
      Token *t = vector_get_token(tokens, i);
      char *t_str = print_token(t);
      printf("%s\n", t_str);
      free(t_str);
    }

    // Clean up
    for (int i = 0; i < tokens->size; i++) {
      Token *t = vector_get_token(tokens, i);
      free_token(t);
    }
    free(tokens->data);
    free(tokens);

    printf("Compilation succeeded: lexical analysis complete\n");
    exit(EXIT_SUCCESS);
  }

  // Parse
  Vector* program = parse(tokens);

  // Clean up tokens
  for (int i = 0; i < tokens->size; i++) {
    Token *t = vector_get_token(tokens, i);
    free_token(t);
  }
  free(tokens->data);
  free(tokens);
}
