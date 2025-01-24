#include "lexer.h"
#include "error.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Lexes a string
int lex_str(const char *src, int i, Token *t) {
  if (src[i] != '"') {
    char *msg = malloc(BUFSIZ);
    sprintf(msg, "Unexpected char '%c' at '%d'\n", src[i], i);
    lex_error(msg);
  }
  t->start = i;

  for (i += 1; i < strlen(src); i++) {
    if (src[i] == '\n') {
      char *msg = malloc(BUFSIZ);
      sprintf(msg, "Unexpected char '%c' at '%d'\n", src[i], i);
      lex_error(msg);
    } else if (src[i] == '"') {
      i += 1;
      t->type = STRING;
      int len = i - t->start;
      t->text = malloc(len + 1);
      memset(t->text, 0, len + 1);
      strncpy(t->text, src + t->start, len);
      return i;
    }
  }
  char *msg = malloc(BUFSIZ);
  sprintf(msg, "Unexpected char '%c' at '%d'\n", src[i], i);
  lex_error(msg);
  return i;
}

// lexes a keyword or variable
int lex_wrd(const char *src, int i, Token *t) {
  if (!isalpha(src[i])) {
    char *msg = malloc(BUFSIZ);
    sprintf(msg, "Unexpected char '%c' at '%d'\n", src[i], i);
    lex_error(msg);
  }
  t->start = i;

  for (i += 1; i < strlen(src); i++) {
    if (isalpha(src[i]) || isdigit(src[i]) || src[i] == '_')
      continue;
    else
      break;
  }
  int len = i - t->start;
  char *wrd = malloc(len + 1);
  memset(wrd, 0, len + 1);
  strncpy(wrd, src + t->start, len);

  t->type = keyword(wrd);
  t->text = wrd;

  return i;
}

// Lexes an INTVAL or FLOATVAL
int lex_num(const char *src, int i, Token *t) {
  if (!isdigit(src[i]) && src[i] != '.') {
    char *msg = malloc(BUFSIZ);
    sprintf(msg, "Unexpected char '%c' at '%d'\n", src[i], i);
    lex_error(msg);
  }

  t->start = i;
  bool dot_found = false;
  bool num_found = false;
  for (; i < strlen(src); i++) {
    if (src[i] == '.') {
      if (dot_found) {
        break;
      }
      dot_found = true;
    } else if (isdigit(src[i]))
      num_found = true;
    else
      break;
  }
  if (dot_found && !num_found)
    t->type = DOT;
  else if (dot_found)
    t->type = FLOATVAL;
  else
    t->type = INTVAL;

  int len = i - t->start;
  t->text = malloc(len + 1);
  memset(t->text, 0, len + 1);
  strncpy(t->text, src + t->start, len);
  return i;
}

// Lexes a new line
int lex_nl(const char *src, int i, Token *t) {
  if (src[i] != '\n') {
    char *msg = malloc(BUFSIZ);
    sprintf(msg, "Unexpected char '%c' at '%d'\n", src[i], i);
    lex_error(msg);
  }
  t->start = i;

  for (i += 1; i < strlen(src); i++) {
    if (src[i] != '\n' && src[i] != ' ' && src[i] != '\\')
      break;
  }
  t->type = NEWLINE;
  return i;
}

// lexes whitespace
int lex_ws(const char *src, int i, Token *t) {
  if (src[i] != ' ') {
    char *msg = malloc(BUFSIZ);
    sprintf(msg, "Unexpected char '%c' at '%d'\n", src[i], i);
    lex_error(msg);
  }
  t->start = i;

  for (i += 1; i < strlen(src); i++) {
    if (src[i] != ' ')
      break;
  }
  t->type = WS;
  return i;
}

// lexes an OP
int lex_op(const char *src, int i, Token *t) {
  t->start = i;
  t->type = OP;

  switch (src[i]) {
  case '=':
    if (i + 1 < strlen(src) && src[i + 1] == '=') {
      t->text = malloc(3);
      memset(t->text, 0, 3);
      strncpy(t->text, src + i, 2);
      i += 2;
    } else {
      t->type = EQUALS;
      t->text = malloc(2);
      memset(t->text, 0, 2);
      strncpy(t->text, src + i, 1);
      i += 1;
    }
    break;
  case '<':
    if (i + 1 < strlen(src) && src[i + 1] == '=') {
      t->text = malloc(3);
      memset(t->text, 0, 3);
      strncpy(t->text, src + i, 2);
      i += 2;
    } else {
      t->text = malloc(2);
      memset(t->text, 0, 2);
      strncpy(t->text, src + i, 1);
      i += 1;
    }
    break;
  case '>':
    if (i + 1 < strlen(src) && src[i + 1] == '=') {
      t->text = malloc(3);
      memset(t->text, 0, 3);
      strncpy(t->text, src + i, 2);
      i += 2;
    } else {
      t->text = malloc(2);
      memset(t->text, 0, 2);
      strncpy(t->text, src + i, 1);
      i += 1;
    }
    break;
  case '!':
    if (i + 1 < strlen(src) && src[i + 1] == '=') {
      t->text = malloc(3);
      memset(t->text, 0, 3);
      strncpy(t->text, src + i, 2);
      i += 2;
    } else {
      t->text = malloc(2);
      memset(t->text, 0, 2);
      strncpy(t->text, src + i, 1);
      i += 1;
    }
    break;
  case '&':
    if (i + 1 < strlen(src) && src[i + 1] == '&') {
      t->text = malloc(3);
      memset(t->text, 0, 3);
      strncpy(t->text, src + i, 2);
      i += 2;
    } else {
      char *msg = malloc(BUFSIZ);
      sprintf(msg, "Unexpected char '%c' at '%d'\n", src[i], i);
      lex_error(msg);
    }
    break;
  case '|':
    if (i + 1 < strlen(src) && src[i + 1] == '|') {
      t->text = malloc(3);
      memset(t->text, 0, 3);
      strncpy(t->text, src + i, 2);
      i += 2;
    } else {
      char *msg = malloc(BUFSIZ);
      sprintf(msg, "Unexpected char '%c' at '%d'\n", src[i], i);
      lex_error(msg);
    }
    break;
  case '/':
    if (i + 1 < strlen(src) && src[i + 1] == '/') {
      for (; i < strlen(src); i++) {
        if (src[i] == '\n')
          break;
      }
      t->type = WS;
    } else if (i + 1 < strlen(src) && src[i + 1] == '*') {
      for (i += 2; i < strlen(src); i++) {
        if (src[i] == '*' && i + 1 < strlen(src) && src[i + 1] == '/')
          break;
      }
      i += 2;
      t->type = WS;
    } else {
      t->type = OP;
      t->text = malloc(2);
      memset(t->text, 0, 2);
      strncpy(t->text, src + i, 1);
      i += 1;
    }
    break;
  case '-':
  case '+':
  case '*':
  case '%':
    t->text = malloc(2);
    memset(t->text, 0, 2);
    strncpy(t->text, src + i, 1);
    i += 1;
    break;
  default:;
    char *msg = malloc(BUFSIZ);
    sprintf(msg, "Unexpected char '%c' at '%d'\n", src[i], i);
    lex_error(msg);
  }
  return i;
}

// lexes single char tokens (pnct | nl | ws | op)
int lex_pnct(const char *src, int i, Token *t) {
  switch (src[i]) {
    // Check punctuation
  case ':':
    t->type = COLON;
    t->text = malloc(2);
    memset(t->text, 0, 2);
    strncpy(t->text, src + i, 1);
    i += 1;
    break;
  case ',':
    t->type = COMMA;
    t->text = malloc(2);
    memset(t->text, 0, 2);
    strncpy(t->text, src + i, 1);
    i += 1;
    break;
  case '.':
    t->type = DOT;
    t->text = malloc(2);
    memset(t->text, 0, 2);
    strncpy(t->text, src + i, 1);
    i += 1;
    break;
  case '{':
    t->type = LCURLY;
    t->text = malloc(2);
    memset(t->text, 0, 2);
    strncpy(t->text, src + i, 1);
    i += 1;
    break;
  case '}':
    t->type = RCURLY;
    t->text = malloc(2);
    memset(t->text, 0, 2);
    strncpy(t->text, src + i, 1);
    i += 1;
    break;
  case '(':
    t->type = LPAREN;
    t->text = malloc(2);
    memset(t->text, 0, 2);
    strncpy(t->text, src + i, 1);
    i += 1;
    break;
  case ')':
    t->type = RPAREN;
    t->text = malloc(2);
    memset(t->text, 0, 2);
    strncpy(t->text, src + i, 1);
    i += 1;
    break;
  case '[':
    t->type = LSQUARE;
    t->text = malloc(2);
    memset(t->text, 0, 2);
    strncpy(t->text, src + i, 1);
    i += 1;
    break;
  case ']':
    t->type = RSQUARE;
    t->text = malloc(2);
    memset(t->text, 0, 2);
    strncpy(t->text, src + i, 1);
    i += 1;
    break;
    // Handle special single char cases
  case '\n':
    i = lex_nl(src, i, t);
    break;
  case '"':
    i = lex_str(src, i, t);
    break;
  case ' ':
    i = lex_ws(src, i, t);
    break;
  case '\\':
    for (i += 1; i < strlen(src); i++) {
      if (src[i] == '\n')
        break;
    }
    i += 1;
    t->type = ESC;
    break;
  case '=':
  case '+':
  case '<':
  case '>':
  case '-':
  case '&':
  case '|':
  case '!':
  case '*':
  case '%':
  case '/':
    i = lex_op(src, i, t);
    break;
  default:;
    char *msg = malloc(BUFSIZ);
    sprintf(msg, "Unexpected char '%c' at '%d'\n", src[i], i);
    lex_error(msg);
  }

  return i;
}

TokenVector *lex(const char *src) {
  // Validate input
  for (int k = 0; k < strlen(src); k++) {
    if (!isprint(src[k]) && src[k] != '\n') {
      char *msg = malloc(BUFSIZ);
      sprintf(msg, "Invalid char%c at%d", ' ', k);
      lex_error(msg);
    }
  }

  TokenVector *tokens = malloc(sizeof(TokenVector));
  memset(tokens, 0, sizeof(TokenVector));
  vector_init_token(tokens, BUFSIZ);

  int i = 0;
  while (i < strlen(src)) {
    Token *t = malloc(sizeof(Token));
    t->start = i;

    // Check letter
    if (isalpha(src[i])) {
      i = lex_wrd(src, i, t);
      vector_append_token(tokens, t);
    }

    // Check number
    else if (isdigit(src[i]) || src[i] == '.') {
      i = lex_num(src, i, t);
      vector_append_token(tokens, t);
    }

    // All other valid chars
    else {
      i = lex_pnct(src, i, t);
      if (t->type == NEWLINE) {
        if (tokens->size > 0 &&
            ((Token *)(tokens->data[tokens->size - 1]))->type == NEWLINE) {
          free(t);
          continue;
        }
      }
      if (t->type == WS || t->type == ESC) {
        free(t);
        continue;
      }
      vector_append_token(tokens, t);
    }
  };
  Token *last = malloc(sizeof(Token));
  last->type = END_OF_FILE;
  last->start = strlen(src) + 1;
  vector_append_token(tokens, last);

  return tokens;
}
