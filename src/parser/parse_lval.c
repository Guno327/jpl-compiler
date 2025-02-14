#include "parse_lval.h"
#include "alloc.h"
#include "ast.h"
#include "parser.h"
#include "vector_get.h"
#include <string.h>

int parse_lvalue(vector *tokens, int i, lval *v) {
  v->start = i;
  expect_token(tokens, i, VARIABLE);
  char *v_var = vector_get_token(tokens, i)->text;
  char *var = alloc(strlen(v_var) + 1);
  memcpy(var, v_var, strlen(v_var));

  if (i < tokens->size - 1 && peek_token(tokens, i + 1) == LSQUARE) {
    array_lval *alv = alloc(sizeof(array_lval));
    alv->start = i;
    alv->var = var;

    i += 2;
    vector *vars = alloc(sizeof(vector));
    vector_init(vars, 8, STRVECTOR);
    while (i < tokens->size) {
      expect_token(tokens, i, VARIABLE);
      char *cur_str = vector_get_token(tokens, i)->text;
      char *cur_v = alloc(strlen(cur_str) + 1);
      memcpy(cur_v, cur_str, strlen(cur_str));
      vector_append(vars, cur_v);
      i += 1;

      if (peek_token(tokens, i) == RSQUARE) {
        i += 1;
        break;
      }
      expect_token(tokens, i, COMMA);
      i += 1;
    }
    alv->vars = vars;

    v->node = alv;
    v->type = ARRAYLVALUE;
  }

  else if (peek_token(tokens, i) == VARIABLE) {
    var_lval *lvl = alloc(sizeof(var_lval));
    lvl->start = i;
    lvl->var = var;

    v->type = VARLVALUE;
    v->node = lvl;
    i += 1;
  }

  return i;
}
