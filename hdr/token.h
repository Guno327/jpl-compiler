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
} TokenType;

typedef struct {
  TokenType type;
  int start;
  char *text;
} Token;

char *print_token(Token *t);
TokenType keyword(char *text);

#endif
