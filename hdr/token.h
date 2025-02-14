#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
  WS,
  ARRAY,
  ASSERT,
  BOOL,
  COLON,
  COMMA,
  DOT,
  ELSE,
  END_OF_FILE,
  EQUALS,
  FALSE,
  FLOAT,
  FLOATVAL,
  FN,
  IF,
  IMAGE,
  INT,
  INTVAL,
  LCURLY,
  LET,
  LPAREN,
  LSQUARE,
  NEWLINE,
  OP,
  PRINT,
  RCURLY,
  READ,
  RETURN,
  RPAREN,
  RSQUARE,
  SHOW,
  STRING,
  STRUCT,
  SUM,
  THEN,
  TIME,
  TO,
  TRUE,
  VARIABLE,
  VOID,
  WRITE
} tokentype;

typedef struct {
  tokentype type;
  int start;
  char *text;
} token;

char *print_token(token *t);
tokentype keyword(char *text);

#endif
