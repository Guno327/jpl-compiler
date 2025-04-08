#include "t.h"
#include "token.h"
#ifndef COMPILER_ERROR_H
#define COMPILER_ERROR_H

void lex_error(char c, long i);
void parse_error(token *t);
void typecheck_error(char *msg, long i);
void ir_error(char *msg);
void set_src(char *src);
void set_verbose(bool mode);

typedef struct {
  long line;
  long i;
} src_location;

void print_src_location(long i);

#endif
