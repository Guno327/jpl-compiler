#include "compiler_error.h"
#include "ast.h"
#include "safe.h"
#include "t.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *src = NULL;
bool verbose = false;

void lex_error(char c, int i) {
  if (verbose)
    print_src_location(i);
  if (c == '\n')
    printf("Compilation Failed [LEXER] (%d): unexpected char NEWLINE\n", i);
  else
    printf("Compilation Failed [LEXER] (%d): unexpected char %c\n", i, c);
  exit(EXIT_FAILURE);
}

void parse_error(token *t) {
  if (verbose)
    print_src_location(t->start);
  char *token = print_token(t);
  printf("Compilation Failed [PARSER] (%d): unexpected token of type %s\n",
         t->start, token);
  exit(EXIT_FAILURE);
}

void typecheck_error(char *msg, int i) {
  if (verbose)
    print_src_location(i);
  printf("Compilation failed [TYPECHECKER] (%d): %s\n", i, msg);
  exit(EXIT_FAILURE);
}

void ir_error(char *msg) {
  printf("Compilation failed [IR]: %s\n", msg);
  exit(EXIT_FAILURE);
}

void set_src(char *s) { src = s; }
void set_verbose(bool mode) { verbose = mode; }

void print_src_location(int i) {
  src_location *loc = safe_alloc(sizeof(src_location));
  int line = 1;
  for (int j = 0; j <= i; j++) {
    if (src[j] == '\n') {
      loc->i = 0;
      line += 1;
    } else {
      loc->i += 1;
    }
  }
  loc->line = line;
  int print_start = line - 1 < 1 ? 1 : line - 1;

  int cur_line = 1;
  printf("%d\t", print_start);
  for (int j = 0; j < strlen(src); j++) {
    if (cur_line == print_start) {
      int print_count = 0;
      for (int k = j; k < strlen(src); k++) {
        if (print_count == 1)
          printf("\033[31m");
        printf("%c", src[k]);
        printf("\033[0m");
        if (src[k] == '\n') {
          print_count += 1;
          if (print_count == 3 || k == strlen(src) - 1)
            break;
          printf("%d\t", print_start + print_count);
        }
      }
      break;
    } else if (src[j] == '\n') {
      cur_line += 1;
    }
  }
  printf("\n");
}
