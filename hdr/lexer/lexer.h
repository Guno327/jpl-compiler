#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include "vector.h"

TokenVector* lex(const char *src);
int lex_str(const char *src, int i, Token *t);
int lex_wrd(const char *src, int i, Token *t);
int lex_num(const char *src, int i, Token *t);
int lex_nl(const char *src, int i, Token *t);
int lex_ws(const char *src, int i, Token *t);
int lex_op(const char *src, int i, Token *t);
int lex_pnct(const char *src, int i, Token *t);

#endif
