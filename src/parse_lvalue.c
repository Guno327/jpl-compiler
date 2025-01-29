#include "parse_lvalue.h"
#include "ast.h"
#include "parser.h"
#include <stdlib.h>
#include <string.h>

int parse_lvalue(TokenVector *tokens, int i, LValue *v) {
  v->start = i;
  v->type = VARLVALUE;
  VarLValue *lvl = malloc(sizeof(VarLValue));
  lvl->start = i;
  expect_token(tokens, i, VARIABLE);
  char *v_var = vector_get_token(tokens, i)->text;
  lvl->var = malloc(strlen(v_var) + 1);
  memset(lvl->var, 0, strlen(v_var) + 1);
  memcpy(lvl->var, v_var, strlen(v_var));
  v->node = lvl;
  return i + 1;
}
