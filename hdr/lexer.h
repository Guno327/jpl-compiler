#include "token.h"
#include "vector.h"
#ifndef LEXER_H
#define LEXER_H

vector *lex(const char *src);
long lex_str(const char *src, long i, token *t);
long lex_wrd(const char *src, long i, token *t);
long lex_num(const char *src, long i, token *t);
long lex_nl(const char *src, long i, token *t);
long lex_ws(const char *src, long i, token *t);
long lex_op(const char *src, long i, token *t);
long lex_pnct(const char *src, long i, token *t);

#endif
