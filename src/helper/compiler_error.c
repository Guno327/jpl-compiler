#include "compiler_error.h"
#include "alloc.h"
#include "ast.h"
#include "t.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *src = NULL;

void lex_error(char c, int i) {
  src_location *loc = get_src_location(i);
  if (c == '\n')
    printf("Compilation Failed [LEXER] (%d:%d): unexpected char NEWLINE\n",
           loc->line, loc->i);
  else
    printf("Compilation Failed [LEXER] (%d:%d): unexpected char %c\n",
           loc->line, loc->i, c);
  exit(EXIT_FAILURE);
}

void parse_error(token *t) {
  src_location *loc = get_src_location(t->start);
  char *token = print_token(t);
  printf("Compilation Failed [PARSER] (%d:%d): unexpected token of type %s\n",
         loc->line, loc->i, token);
  exit(EXIT_FAILURE);
}

void typecheck_error(char *msg, int i) {
  src_location *loc = get_src_location(i);
  printf("Compilation failed [TYPECHECKER] (%d:%d): %s\n", loc->line, loc->i,
         msg);
  exit(EXIT_FAILURE);
}

void set_src(char *s) { src = s; }

src_location *get_src_location(int i) {
  src_location *loc = alloc(sizeof(src_location));
  loc->line = 0;
  loc->i = 0;
  for (int j = 0; j < i + 1; j++) {
    if (src[j] == '\n') {
      loc->line += 1;
      loc->i = 0;
    } else {
      loc->i += 1;
    }
  }
  return loc;
}
