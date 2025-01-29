#include "parse_lvalue.h"
#include "alloc.h"
#include "ast.h"
#include "compiler_error.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parse_lvalue(TokenVector *tokens, int i, LValue *v) {
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
    int count = 0;
    char **vars = NULL;
    while (i < tokens->size) {
      switch (peek_token(tokens, i)) {
      case VARIABLE:
        count += 1;
        char **tmp = NULL;
        while (tmp == NULL)
          tmp = realloc(vars, sizeof(char *) * count);
        vars = tmp;

        char *str = vector_get_token(tokens, i)->text;
        int len = strlen(str);
        char *cur_var = alloc(len + 1);
        memcpy(cur_var, str, len);
        vars[count - 1] = cur_var;

        i += 1;
        break;
      case COMMA:
        i += 1;
        continue;
      case RSQUARE:
        i += 1;
        goto finish;
      default:;
        char *msg = alloc(BUFSIZ);
        sprintf(msg, "Unexpected token '%s' at '%d'",
                vector_get_token(tokens, i)->text, i);
        parse_error(msg);
      }
    }
  finish:
    alv->vars = vars;
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
