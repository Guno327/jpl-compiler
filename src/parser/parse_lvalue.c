#include "parse_lvalue.h"
#include "alloc.h"
#include "ast.h"
#include "parser.h"
#include <string.h>

int parse_lvalue(Vector *tokens, int i, LValue *v) {
  v->start = i;
  expect_token(tokens, i, VARIABLE);
  char *v_var = vector_get_token(tokens, i)->text;
  char *var = alloc(strlen(v_var) + 1);
  memcpy(var, v_var, strlen(v_var));

  if (i < tokens->size - 1 && peek_token(tokens, i + 1) == LSQUARE) {
    ArrayLValue *alv = alloc(sizeof(ArrayLValue));
    alv->start = i;
    alv->var = var;

    i += 2;
    Vector *vars = alloc(sizeof(Vector));
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
    alv->vars = (char **)vars->data;
    alv->vars_size = vars->size;

    v->node = alv;
    v->type = ARRAYLVALUE;
  }

  else if (peek_token(tokens, i) == VARIABLE) {
    VarLValue *lvl = alloc(sizeof(VarLValue));
    lvl->start = i;
    lvl->var = var;

    v->type = VARLVALUE;
    v->node = lvl;
    i += 1;
  }

  return i;
}
