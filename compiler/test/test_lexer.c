#include <stdlib.h>
#include <check.h>
#include "test.h"
#include "../src/lexer.h"

void test_lexer() {
  char* src = NULL;
  Token* t = malloc(sizeof(Token));
  int ret = 0;
  //test lex_str
  src = "\"test\"";
  ret = lex_str(src, 0, t);
  ck_assert_msg(ret == 5, "lex_str:1.1");
  ck_assert_msg(t->start == 0, "lex_str:1.2");
  ck_assert_msg(t->type == STRING, "lex_str:1.3");
  ck_assert_msg(!strcmp(t->text, src), "lex_str:1.4");
  
  //test lex_wrd
  
  //test lex_num
  
  //test lex_nl
  
  //test lex_ws
  
  //test lex_op
  
  //test lex_pnct
  
  //test lex
  return;
}
