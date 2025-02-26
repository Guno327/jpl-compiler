#include "t.h"
#include "token.h"
#ifndef COMPILER_ERROR_H
#define COMPILER_ERROR_H

void lex_error(char c, int i);
void parse_error(token *t);
void typecheck_error(char *msg, int i);
void ir_error(char *msg);
void set_src(char *src);
void set_verbose(bool mode);

typedef struct {
  int line;
  int i;
} src_location;

void print_src_location(int i);

#endif
