#include "token.h"
#include "vector.h"
#ifndef LEXER_H
#define LEXER_H

vector *lex(const char *src);
int lex_str(const char *src, int i, token *t);
int lex_wrd(const char *src, int i, token *t);
int lex_num(const char *src, int i, token *t);
int lex_nl(const char *src, int i, token *t);
int lex_ws(const char *src, int i, token *t);
int lex_op(const char *src, int i, token *t);
int lex_pnct(const char *src, int i, token *t);

#endif
