#include "asm_ir.h"
#include "c_ir.h"
#include "compiler_error.h"
#include "lexer.h"
#include "parser.h"
#include "safe.h"
#include "typecheck.h"
#include "vector.h"
#include "vector_get.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { LEX, PARSE, TYPECHECK, C_IR, ASM_IR, ALL } RunMode;
int opt = 0;

int main(int argc, char **argv) {
  RunMode mode = ASM_IR;

  // Handle args
  if (argc < 2) {
    fprintf(stderr, "No file given\n");
    exit(EXIT_FAILURE);
  };

  char *filename = NULL;
  for (long i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-l"))
      mode = LEX;
    else if (!strcmp(argv[i], "-p"))
      mode = PARSE;
    else if (!strcmp(argv[i], "-t"))
      mode = TYPECHECK;
    else if (!strcmp(argv[i], "-i"))
      mode = C_IR;
    else if (!strcmp(argv[i], "-s"))
      mode = ASM_IR;
    else if (!strcmp(argv[i], "-v"))
      set_verbose(true);
    else if (!strcmp(argv[i], "-O1"))
      opt = 1;
    else if (!strcmp(argv[i], "-O3"))
      opt = 3;
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
  char *src = safe_alloc(BUFSIZ);
  char *buf = safe_alloc(BUFSIZ);

  long read = 0;
  char *ptr = NULL;

  do {
    ptr = fgets(buf, BUFSIZ, src_file);
    if (ptr == NULL) {
      if (feof(src_file))
        break;
      else {
        fprintf(stderr, "Compilation failed: Error reading file at %ld\n",
                strlen(src));
        exit(EXIT_FAILURE);
      }
    }
    read = strlen(ptr);
    src = safe_realloc_str(src, strlen(src) + read + 1);

    if (src == NULL) {
      fprintf(stderr, "Malloc failed\n");
      exit(EXIT_FAILURE);
    }

    strncat(src, ptr, read);
    memset(ptr, 0, BUFSIZ);
    ptr = NULL;
  } while (read != 0);
  fclose(src_file);

  // Save for error handling
  set_src(src);

  // Lex
  vector *tokens = lex(src);

  if (mode == LEX) {
    for (long i = 0; i < tokens->size; i++) {
      token *t = vector_get_token(tokens, i);
      char *t_str = print_token(t);
      printf("%s\n", t_str);
    }

    printf("Compilation succeeded: lexical analysis complete\n");
    exit(EXIT_SUCCESS);
  }

  // Parse
  vector *program = parse(tokens);

  // Typecheck
  if (mode != PARSE) {
    ctx *global = typecheck(program);

    // C IR
    if (mode == C_IR) {
      c_prog *c_program = gen_c_ir(program, global);
      char *c_program_str = c_prog_to_str(c_program);
      printf("%s\n", c_program_str);
      printf("Compilation succeeded: Conversion to C IR complete\n");
      exit(EXIT_SUCCESS);
    }

    // ASM IR
    if (mode == ASM_IR) {
      asm_prog *asm_program = gen_asm_ir(program, global);
      char *asm_program_str = asm_prog_to_str(asm_program);
      printf("%s\n", asm_program_str);
      printf("Compilation succeeded: Conversion to ASM IR complete\n");
      exit(EXIT_SUCCESS);
    }
  }

  // Print
  if (mode != ALL) {
    for (long i = 0; i < program->size; i++) {
      printf("%s\n", cmd_to_str(vector_get_cmd(program, i)));
    }
    printf("Compilation succeeded: parsing complete\n");

    exit(EXIT_SUCCESS);
  }

  exit(EXIT_SUCCESS);
}
