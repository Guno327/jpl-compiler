#include "token.h"
#include "safe.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

tokentype keyword(char *text) {
  tokentype ret = 0;
  int len = strlen(text) + 1;
  char *wrd_upper = safe_alloc(len);
  for (int j = 0; j < strlen(text); j++)
    wrd_upper[j] = toupper(text[j]);

  if (!strcmp(wrd_upper, "ARRAY"))
    ret = ARRAY;
  else if (!strcmp(wrd_upper, "ASSERT"))
    ret = ASSERT;
  else if (!strcmp(wrd_upper, "BOOL"))
    ret = BOOL;
  else if (!strcmp(wrd_upper, "ELSE"))
    ret = ELSE;
  else if (!strcmp(wrd_upper, "FALSE"))
    ret = FALSE;
  else if (!strcmp(wrd_upper, "FLOAT"))
    ret = FLOAT;
  else if (!strcmp(wrd_upper, "FN"))
    ret = FN;
  else if (!strcmp(wrd_upper, "IF"))
    ret = IF;
  else if (!strcmp(wrd_upper, "IMAGE"))
    ret = IMAGE;
  else if (!strcmp(wrd_upper, "INT"))
    ret = INT;
  else if (!strcmp(wrd_upper, "LET"))
    ret = LET;
  else if (!strcmp(wrd_upper, "PRINT"))
    ret = PRINT;
  else if (!strcmp(wrd_upper, "READ"))
    ret = READ;
  else if (!strcmp(wrd_upper, "RETURN"))
    ret = RETURN;
  else if (!strcmp(wrd_upper, "SHOW"))
    ret = SHOW;
  else if (!strcmp(wrd_upper, "STRUCT"))
    ret = STRUCT;
  else if (!strcmp(wrd_upper, "SUM"))
    ret = SUM;
  else if (!strcmp(wrd_upper, "THEN"))
    ret = THEN;
  else if (!strcmp(wrd_upper, "TIME"))
    ret = TIME;
  else if (!strcmp(wrd_upper, "TO"))
    ret = TO;
  else if (!strcmp(wrd_upper, "TRUE"))
    ret = TRUE;
  else if (!strcmp(wrd_upper, "VOID"))
    ret = VOID;
  else if (!strcmp(wrd_upper, "WRITE"))
    ret = WRITE;
  else
    ret = VARIABLE;

  free(wrd_upper);
  return ret;
}

char *print_token(token *t) {
  long len = BUFSIZ + (sizeof(t->text));
  char *token_str = safe_alloc(len);
  switch (t->type) {
  case ARRAY:
    sprintf(token_str, "ARRAY '%s'", t->text);
    break;
  case ASSERT:
    sprintf(token_str, "ASSERT '%s'", t->text);
    break;
  case BOOL:
    sprintf(token_str, "BOOL '%s'", t->text);
    break;
  case COLON:
    sprintf(token_str, "COLON '%s'", t->text);
    break;
  case COMMA:
    sprintf(token_str, "COMMA '%s'", t->text);
    break;
  case DOT:
    sprintf(token_str, "DOT '%s'", t->text);
    break;
  case ELSE:
    sprintf(token_str, "ELSE '%s'", t->text);
    break;
  case END_OF_FILE:
    sprintf(token_str, "END_OF_FILE");
    break;
  case EQUALS:
    sprintf(token_str, "EQUALS '%s'", t->text);
    break;
  case FALSE:
    sprintf(token_str, "FALSE '%s'", t->text);
    break;
  case FLOAT:
    sprintf(token_str, "FLOAT '%s'", t->text);
    break;
  case FLOATVAL:
    sprintf(token_str, "FLOATVAL '%s'", t->text);
    break;
  case FN:
    sprintf(token_str, "FN '%s'", t->text);
    break;
  case IF:
    sprintf(token_str, "IF '%s'", t->text);
    break;
  case IMAGE:
    sprintf(token_str, "IMAGE '%s'", t->text);
    break;
  case INT:
    sprintf(token_str, "INT '%s'", t->text);
    break;
  case INTVAL:
    sprintf(token_str, "INTVAL '%s'", t->text);
    break;
  case LCURLY:
    sprintf(token_str, "LCURLY '%s'", t->text);
    break;
  case LET:
    sprintf(token_str, "LET '%s'", t->text);
    break;
  case LPAREN:
    sprintf(token_str, "LPAREN '%s'", t->text);
    break;
  case LSQUARE:
    sprintf(token_str, "LSQUARE '%s'", t->text);
    break;
  case NEWLINE:
    sprintf(token_str, "NEWLINE");
    break;
  case OP:
    sprintf(token_str, "OP '%s'", t->text);
    break;
  case PRINT:
    sprintf(token_str, "PRINT '%s'", t->text);
    break;
  case RCURLY:
    sprintf(token_str, "RCURLY '%s'", t->text);
    break;
  case READ:
    sprintf(token_str, "READ '%s'", t->text);
    break;
  case RETURN:
    sprintf(token_str, "RETURN '%s'", t->text);
    break;
  case RPAREN:
    sprintf(token_str, "RPAREN '%s'", t->text);
    break;
  case RSQUARE:
    sprintf(token_str, "RSQUARE '%s'", t->text);
    break;
  case SHOW:
    sprintf(token_str, "SHOW '%s'", t->text);
    break;
  case STRING:
    sprintf(token_str, "STRING '%s'", t->text);
    break;
  case STRUCT:
    sprintf(token_str, "STRUCT '%s'", t->text);
    break;
  case SUM:
    sprintf(token_str, "SUM '%s'", t->text);
    break;
  case THEN:
    sprintf(token_str, "THEN '%s'", t->text);
    break;
  case TIME:
    sprintf(token_str, "TIME '%s'", t->text);
    break;
  case TO:
    sprintf(token_str, "TO '%s'", t->text);
    break;
  case TRUE:
    sprintf(token_str, "TRUE '%s'", t->text);
    break;
  case VARIABLE:
    sprintf(token_str, "VARIABLE '%s'", t->text);
    break;
  case VOID:
    sprintf(token_str, "VOID '%s'", t->text);
    break;
  case WRITE:
    sprintf(token_str, "WRITE '%s'", t->text);
    break;
  case WS:
    printf("Print Error: THIS SHOULD NEVER HAPPEN\n");
    exit(EXIT_FAILURE);
  }
  return token_str;
}
