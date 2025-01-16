#ifndef TOKEN_H
#define TOKEN_H

#define ESC -2
#define WS -1
#define ARRAY 1
#define ASSERT 2
#define BOOL 3
#define COLON 4
#define COMMA 5
#define DOT 6
#define ELSE 7
#define END_OF_FILE 8
#define EQUALS 9
#define FALSE 10
#define FLOAT 11
#define FLOATVAL 12
#define FN 13
#define IF 14
#define IMAGE 15
#define INT 16
#define INTVAL 17
#define LCURLY 18
#define LET 19
#define LPAREN 20
#define LSQUARE 21
#define NEWLINE 22
#define OP 23
#define PRINT 24
#define RCURLY 25
#define READ 26
#define RETURN 27
#define RPAREN 28
#define RSQUARE 29
#define SHOW 30
#define STRING 31
#define STRUCT 32
#define SUM 33
#define THEN 34
#define TIME 35
#define TO 36
#define TRUE 37
#define VARIABLE 38
#define VOID 39
#define WRITE 40

typedef struct {
  int type;
  int start;
  char *text;
} Token;

char *print_token(Token *t);
int keyword(char *text);

#endif
