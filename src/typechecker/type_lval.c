#include "alloc.h"
#include "compiler_error.h"
#include "ctx.h"
#include "typecheck.h"
#include "vector.h"
#include "vector_get.h"
#include <stdio.h>

void type_lval(lval *lv, t *type, ctx *c) {
  switch (lv->type) {
  case VARLVALUE:;
    var_lval *vlv = (var_lval *)lv->node;
    var_info *vlv_info = alloc(sizeof(var_info));
    vlv_info->name = vlv->var;
    vlv_info->type = type;
    vector_append(c->vars, vlv_info);
    break;
  case ARRAYLVALUE:;
    if (type->type != ARRAY_T) {
      char *msg = alloc(BUFSIZ);
      sprintf(msg, "Expected (ArrayType) got %s", t_to_str(type));
      typecheck_error(msg, lv->start);
    }

    array_lval *alv = (array_lval *)lv->node;
    array_info *alv_info = alloc(sizeof(array_info));
    alv_info->name = alv->var;
    alv_info->rank = alv->vars->size;
    alv_info->type = type;
    vector_append(c->arrays, alv_info);

    for (int i = 0; i < alv->vars->size; i++) {
      var_info *v_info = alloc(sizeof(var_info));
      v_info->name = vector_get_str(alv->vars, i);
      v_info->type = type;
      vector_append(c->vars, v_info);
    }
  }
}
