#include "alloc.h"
#include "compiler_error.h"
#include "ctx.h"
#include "typecheck.h"
#include "vector.h"
#include "vector_get.h"
#include <stdio.h>
#include <string.h>

void type_lval(lval *lv, t *type, ctx *c) {
  switch (lv->type) {
  case VARLVALUE:;
    var_lval *vlv = (var_lval *)lv->node;

    // Check if already defined
    info *vlv_exist = check_ctx(c, vlv->var);
    if (vlv_exist != NULL) {
      char *msg = alloc(BUFSIZ);
      sprintf(msg, "Symbol '%s' already defined", vlv->var);
      typecheck_error(msg, vlv->start);
    }

    // Add new var to scope
    var_info *vi = alloc(sizeof(var_info));
    vi->t = type;
    vi->name = vlv->var;
    vector_append(c->vars, vi);
    break;
  case ARRAYLVALUE:
    // Make sure RHS is an array
    if (type->type != ARRAY_T) {
      char *msg = alloc(BUFSIZ);
      sprintf(msg, "Expected expression of (ArrayType) got %s", t_to_str(type));
      typecheck_error(msg, lv->start);
    }

    // Make sure RHS rank matches LHS definition
    array_lval *alv = (array_lval *)lv->node;
    array_info *ai = (array_info *)type->info;
    if (ai->rank != alv->vars->size) {
      char *msg = alloc(BUFSIZ);
      sprintf(msg, "Expected array of rank %lu got one of %d", alv->vars->size,
              ai->rank);
      typecheck_error(msg, lv->start);
    }
    ai->name = alv->var;

    // Check if already defined
    info *alv_exist = check_ctx(c, alv->var);
    if (alv_exist != NULL) {
      char *msg = alloc(BUFSIZ);
      sprintf(msg, "Symbol '%s' already defined", alv->var);
      typecheck_error(msg, alv->start);
    }

    // Add array to scope
    vector_append(c->arrays, ai);

    // Add all rank definitions to scope
    t *int_t = alloc(sizeof(t));
    int_t->type = INT_T;
    for (int i = 0; i < alv->vars->size; i++) {
      char *var = vector_get_str(alv->vars, i);
      info *exists = check_ctx(c, var);
      if (exists != NULL) {
        char *msg = alloc(BUFSIZ);
        sprintf(msg, "Symbol '%s' already defined", alv->var);
        typecheck_error(msg, alv->start);
      }
      var_info *info = alloc(sizeof(var_info));
      info->name = var;
      info->t = int_t;
      vector_append(c->vars, info);
    }
    break;
  }
}
