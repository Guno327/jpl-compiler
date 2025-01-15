#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "lexer.h"

// Lexes a string
int lex_str(const char* src, int i, Token* t){
  if(src[i] != '"'){
      fprintf(stderr, "Unexpected char '%c' at %d in string\n", src[i], i);
      exit(EXIT_FAILURE);
  }
  t->start = i;

  int start = strlen(src);
  for(i += 1; i < strlen(src); i++){
    if (src[i] == '\n'){
      fprintf(stderr, "Unexpected newline at %d in string\n", i);
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
  fprintf(stderr, "Unexpectely encountered EOF while parsing in string\n");
  exit(EXIT_FAILURE);
}

//lexes a keyword or variable
int lex_wrd(const char* src, int i, Token* t){
  if (!isalpha(src[i])){
    fprintf(stderr, "Unexpected char '%c' at %d", src[i], i);
    exit(EXIT_FAILURE);
  }
  t->start = i;
  
  for(i += 1; i < strlen(src); i++){
    if (isalpha(src[i]) || isdigit(src[i]) || src[i] == '_')
      continue;
    else
      break;
  }
  int len = i - t->start;
  char* wrd = malloc(len + 1);
  strncpy(wrd, src+t->start, len);
  
  t->type = keyword(wrd);
  t->text = wrd;
  
  return i;
}

// Lexes an INTVAL or FLOATVAL
int lex_num(const char* src, int i, Token* t){
  if (!isdigit(src[i]) && src[i] != '.'){
      fprintf(stderr, "Unexpected char '%c' at %d in number\n", src[i], i);
      exit(EXIT_FAILURE);
  }

  t->start = i;
  bool dot_found = false;
  bool num_found = false;
  for(;i < strlen(src); i++){
    if (src[i] == '.'){
      if (dot_found){
        fprintf(stderr, "Unexpected char '%c' at %d in floatval\n", src[i], i);
        exit(EXIT_FAILURE);
      }
      dot_found = true;
    }
    else if (isdigit(src[i]))
      num_found = true;
    else
      break;
  }
  if (dot_found && !num_found){
    fprintf(stderr, "Unexpected char '%c' at %d in floatval\n", src[i], i);
    exit(EXIT_FAILURE);
  }
  else if (dot_found)
    t->type = FLOATVAL;
  else
    t->type = INTVAL;
  
  t->text = malloc(i - t->start + 1);
  strncpy(t->text, src + t->start, i);
  return i;
}

// Lexes a new line
int lex_nl(const char* src, int i, Token* t){
  if (src[i] != '\n'){
      fprintf(stderr, "Unexpected char '%c' at %d in newline\n", src[i], i);
      exit(EXIT_FAILURE);
  }
  t->start = i;

  for(i+=1; i < strlen(src); i++){
    if (src[i] != '\n' && src[i] != ' ' && src[i] != '\\')
      break;
  }
  t->type = NEWLINE;
  return i;
}

//lexes whitespace
int lex_ws(const char* src, int i, Token* t){
  if (src[i] != ' ' && src[i] != '\\'){
      fprintf(stderr, "Unexpected char '%c' at %d in whitespace\n", src[i], i);
      exit(EXIT_FAILURE);
  }
  t->start = i;

  for(i+=1; i < strlen(src); i++){
    if (src[i] != ' ' && src[i] != '\\')
      break;
  }
  t->type = -1;
  return i;
}

//lexes an OP
int lex_op(const char* src, int i, Token* t){
  t->start = i;
  t->type = OP;

  switch (src[i]){
    case '=':
      if (i+1 < strlen(src) && src[i+1] == '='){
        t->text = "==";
        i += 2;
      }
      else{
        t->text = "=";
        i += 1;
      }
      break;
    case '<':
      if (i+1 < strlen(src) && src[i+1] == '='){
        t->text = "<=";
        i += 2;
      }
      else{
        t->text = "<";
        i += 1;
      }
      break;
    case '>':
      if (i+1 < strlen(src) && src[i+1] == '='){
        t->text = ">=";
        i += 2;
      }
      else {
        t->text = ">";
        i += 1;
      }
      break;
    case '!':
      if (i+1 < strlen(src) && src[i+1] == '='){
        t->text = "!=";
        i += 2;
      }
      else {
        t->text = "!";
        i += 1;
      }
      break;
    case '&':
      if (i+1 < strlen(src) && src[i+1] == '&'){
        t->text = "&&";
        i += 2;
      }
      else {
        fprintf(stderr, "Unexpected char '%c' at %d in op\n", src[i], i);
        exit(EXIT_FAILURE);
      }
      break;
    case '|':
      if (i+1 < strlen(src) && src[i+1] == '|'){
        t->text = "||";
        i += 2;
      }
      else {
        fprintf(stderr, "Unexpected char '%c' at %d in op\n", src[i], i);
        exit(EXIT_FAILURE);
      }
      break;
    case '/':
      if (i+1 < strlen(src) && src[i+1] == '/'){
        for(; i<strlen(src); i++){
          if (src[i] == '\n')
            break;
        }
        t->type = -1;
      }
      else if (i+1 < strlen(src) && src[i+1] == '*'){
        for(i+=2 ; i < strlen(src); i++){
          if (src[i] == '*' && i+1 < strlen(src) && src[i+1] == '/')
            break;
        }
        i += 2;
        t->type = -1;
      }
      else {
        t->type = OP;
        t->text = "/";
        i += 1;
      }
      break;
    case '-':
    case '+':
    case '*':
    case '%':
      t->text = malloc(2);
      strncpy(t->text, src + i, 1);
      i += 1;
      break;
    default:
      fprintf(stderr, "Unexpected char '%c' at %d in op\n", src[i], i);
      exit(EXIT_FAILURE);
  }
  return i;
}

//lexes single char tokens (pnct | nl | ws | op)
int lex_pnct(const char* src, int i, Token* t){
  switch (src[i]){
  // Check punctuation
    case ':': 
      t->type = COLON; 
      t->text = ":"; 
      i = 1;
      break; 
    case ',': 
      t->type = COMMA; 
      t->text = ",";
      i = 1;
      break;
    case '.': 
      t->type = DOT; 
      t->text = "."; 
      i = 1;
      break;
    case '{': 
      t->type = LCURLY; 
      t->text = "{"; 
      i = 1;
      break;
    case '}': 
      t->type = RCURLY; 
      t->text = "}"; 
      i = 1;
      break;
    case '(': 
      t->type = LPAREN; 
      t->text = "("; 
      i = 1;
      break;
    case ')': 
      t->type = RPAREN; 
      t->text = ")"; 
      i = 1;
      break;
    case '[': 
      t->type = LSQUARE;
      t->text = "[";
      i = 1;
      break;
    case ']': 
      t->type = RSQUARE;
      t->text = "]"; 
      i = 1;
      break;
  // Handle special single char cases
    case '\n':
      i = lex_nl(src, i, t);
      break;
    case '"':
      i = lex_str(src, i, t);
      break;
    case ' ':
    case '\\':
      i = lex_ws(src, i, t);
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
    default:
      fprintf(stderr, "Unexpected char '%c' at %d in op\n", src[i], i);
      exit(EXIT_FAILURE);
  }

  return i;
}

Vector* lex(const char* src) {

  Vector* tokens = malloc(sizeof(Vector));
  vector_init(tokens, BUFSIZ);
 
  int i = 0;
  while(i < strlen(src)){
    Token* t = malloc(sizeof(Token));
    t->start = i;

    // Check letter
    if (isalpha(src[i])){
      i = lex_wrd(src, i, t);
      vector_append(tokens, t);
      continue;
    }

    // Check number
    else if (isdigit(src[i]) || src[i] == '.'){
      i = lex_num(src, i, t);
      vector_append(tokens, t);
      continue;
    }
   
    // All other
    else {
      i = lex_pnct(src, i, t);
      if (t->type != -1)
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
