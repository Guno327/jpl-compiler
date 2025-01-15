#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

/** 
 * @brief Lexes a string token from src.
 * 
 * This function traverses through src starting at index i.
 * It expect index i to be a '=' as that signifies the beginning of
 * a string token. It then continues until it find the corresponding
 * '"' to end the string, or encounters a new-line or EOF (error)
 * 
 * @param src The string containing the source code to be lexed
 * @param i The index to begin lexing a string at
 * @param t A pointer to a token where to result will be stored
 * @return The ending index of the string (t implicitly updated in function)
 */
int lex_str(const char* src, int i, Token* t){
  if(src[i] != '"'){
      fprintf(stderr, "Unexpected char '%c' at %d\n", src[i], i);
      exit(EXIT_FAILURE);
  }
  t->start = i;

  int start = strlen(src);
  for(i += 1; i < strlen(src); i++){
    if (src[i] == '\n'){
      fprintf(stderr, "Unexpected newline at %d\n", i);
      exit(EXIT_FAILURE);
    }
    else if (src[i] == '"'){
      t->type = STRING;
      int len = i - t->start;
      t->text = malloc(i - t->start + 2);
      strncpy(t->text, src + t->start, i - t->start + 1);
      return i;
    }
  } 
  fprintf(stderr, "Unexpectely encountered EOF while parsing\n");
  exit(EXIT_FAILURE);
}

/*
  * @breif Lexes a variable of keyword from src
  *
  * This funtion traverses through src starting at index i.
  * It expects src[i] to be a valid alphabet character.
  * Continues parsing until space, EOF, or newline occurs.
  *
  * @param src The string containing the source code to be lexed
  * @param i The index to begin lexing at
  * @param t A pointer to a token where the result will be stored
  * @return The ending index of the token text (t implicitly updated)
*/ 
int lex_wrd(const char* src, int i, Token* t){
  if (!isalpha(src[i])){
    fprintf(stderr, "Unexpected char '%c' at %d", src[i], i);
    exit(EXIT_FAILURE);
  }
  t->start = i;
  
  for(i += 1; i < strlen(src); i++){
    if (isalpha(src[i]) || isdigit(src[i]) || src[i] == '_')
      continue;
    else if (src[i] == ' ' || src[i] == '\\'  || src[i] == '\n')
      break;
    else{
      fprintf(stderr, "Unexpected char '%c' at %d\n", src[i], i);
      exit(EXIT_FAILURE);
    }
  }
  int len = i - t->start;
  char* wrd = malloc(len + 1);
  strncpy(wrd, src+t->start, len);
  
  t->type = keyword(wrd);
  t->text = wrd;
  
  return i;
}

int lex_num(const char* src, int i, Token* t){
  return 0;
}

int lex_nl(const char* src, int i, Token* t){
  return 0;
}

int lex_ws(const char* src, int i, Token* t){
  return 0;
}

int lex_op(const char* src, int i, Token* t){
  return 0;
}

int lex_pnct(const char* src, int i, Token* t){
  int chars = 0;

  switch (src[i]){
  // Check punctuation
    case ':': 
      t->type = COLON; 
      t->text = ":"; 
      chars = 1;
      break; 
    case ',': 
      t->type = COMMA; 
      t->text = ",";
      chars = 1;
      break;
    case '.': 
      t->type = DOT; 
      t->text = "."; 
      chars = 1;
      break;
    case '{': 
      t->type = LCURLY; 
      t->text = "{"; 
      chars = 1;
      break;
    case '}': 
      t->type = RCURLY; 
      t->text = "}"; 
      chars = 1;
      break;
    case '(': 
      t->type = LPAREN; 
      t->text = "("; 
      chars = 1;
      break;
    case ')': 
      t->type = RPAREN; 
      t->text = ")"; 
      chars = 1;
      break;
    case '[': 
      t->type = LSQUARE;
      t->text = "[";
      chars = 1;
      break;
    case ']': 
      t->type = RSQUARE;
      t->text = "]"; 
      chars = 1;
      break;
  // Handle special single char cases
    case '\n':
      chars = lex_nl(src, i, t);
      break;
    case '"':
      chars = lex_str(src, i, t);
      break;
    case ' ':
      chars = lex_ws(src, i, t);
      break;
    case '=':
    case '+':
    case '<':
    case '>':
    case '-':
    case '&':
    case '|':
    case '!':
      chars = lex_op(src, i, t);
      break;
    default:
      fprintf(stderr, "Unexpected char '%c' at %d\n", src[i], i);
      exit(EXIT_FAILURE);
  }

  return chars;
}

Vector* lex(const char* src) {

  Vector* tokens = NULL;
  tokens = malloc(sizeof(Vector));
  
  for (int i = 0; i < strlen(src); i++){
    Token* t = malloc(sizeof(Token));
    t->start = i;

    // Check letter
    if (isalpha(src[i])){
      i += lex_wrd(src, i, t);
      vector_append(tokens, t);
      continue;
    }

    // Check number
    else if (isdigit(src[i])){
      i += lex_num(src, i, t);
      vector_append(tokens, t);
      continue;
    }

    if (t->type != 0){
      vector_append(tokens, t);
      continue;
    }

  };
  Token* last = malloc(sizeof(Token));
  last->type = END_OF_FILE;
  last->start = strlen(src) + 1;
  vector_append(tokens, last);

  return tokens;
}
