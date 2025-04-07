#include "parse_lval.h"
#include "safe.h"
#include "ast.h"
#include "parser.h"
#include "vector_get.h"
#include <string.h>

long parse_lvalue(vector *tokens, long i, lval *v) {
  v->start = vector_get_token(tokens, i)->start;
  expect_token(tokens, i, VARIABLE);
  char *v_var = vector_get_token(tokens, i)->text;
  char *var = safe_alloc(strlen(v_var) + 1);
  memcpy(var, v_var, strlen(v_var));

  if (i < tokens->size - 1 && peek_token(tokens, i + 1) == LSQUARE) {
    array_lval *alv = safe_alloc(sizeof(array_lval));
    alv->start = vector_get_token(tokens, i)->start;
    alv->var = var;

    i += 2;
    vector *vars = safe_alloc(sizeof(vector));
    vector_init(vars, 8, STRVECTOR);
    while (i < tokens->size) {
      expect_token(tokens, i, VARIABLE);
      char *cur_str = vector_get_token(tokens, i)->text;
      char *cur_v = safe_alloc(strlen(cur_str) + 1);
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
    var_lval *lvl = safe_alloc(sizeof(var_lval));
    lvl->start = vector_get_token(tokens, i)->start;
    lvl->var = var;

    v->type = VARLVALUE;
    v->node = lvl;
    i += 1;
  }

  return i;
}
