#include "compiler_error.h"
#include "ctx.h"
#include "safe.h"
#include "typecheck.h"
#include "vector.h"
#include "vector_get.h"
#include <stdio.h>
#include <string.h>

t *typeof_expr(expr *e, ctx *c) {
  t *result = safe_alloc(sizeof(t));
  switch (e->type) {
  case INTEXPR:
    result->type = INT_T;
    result->info = NULL;
    break;
  case FLOATEXPR:
    result->type = FLOAT_T;
    result->info = NULL;
    break;
  case TRUEEXPR:
    result->type = BOOL_T;
    result->info = NULL;
    break;
  case FALSEEXPR:
    result->type = BOOL_T;
    result->info = NULL;
    break;
  case VOIDEXPR:
    result->type = VOID_T;
    result->info = NULL;
    break;
  case BINOPEXPR:;
    binop_expr *bop_expr = (binop_expr *)e->node;
    t *lhs_t = typeof_expr(bop_expr->lhs, c);
    t *rhs_t = typeof_expr(bop_expr->rhs, c);

    // check LHS is valid for op
    switch (bop_expr->op) {
    case ADDOP:
    case SUBOP:
    case MULTOP:
    case DIVOP:
    case MODOP:
      if (lhs_t->type != FLOAT_T && lhs_t->type != INT_T) {
        char *msg = safe_alloc(BUFSIZ);
        sprintf(msg, "Expected type of (IntType) or (FloatType) got %s",
                t_to_str(lhs_t));
        typecheck_error(msg, bop_expr->start);
      }
      result->type = lhs_t->type;
      result->info = NULL;
      break;
    case LTOP:
    case LEOP:
    case GTOP:
    case GEOP:
      if (lhs_t->type != FLOAT_T && lhs_t->type != INT_T) {
        char *msg = safe_alloc(BUFSIZ);
        sprintf(msg, "Expected type of (IntType) or (FloatType) got %s",
                t_to_str(lhs_t));
        typecheck_error(msg, bop_expr->start);
      }
      result->type = BOOL_T;
      result->info = NULL;
      break;
    case ANDOP:
    case OROP:
      if (lhs_t->type != BOOL_T) {
        char *msg = safe_alloc(BUFSIZ);
        sprintf(msg, "Expected type of (BoolType) got %s", t_to_str(lhs_t));
        typecheck_error(msg, bop_expr->start);
      }
      result->type = BOOL_T;
      result->info = NULL;
      break;
    case EQOP:
    case NEOP:
      if (lhs_t->type != FLOAT_T && lhs_t->type != INT_T &&
          lhs_t->type != BOOL_T) {
        char *msg = safe_alloc(BUFSIZ);
        sprintf(msg,
                "Expected type of (BoolType), (FloatType), or (IntType) got %s",
                t_to_str(lhs_t));
        typecheck_error(msg, bop_expr->start);
      }
      result->type = BOOL_T;
      result->info = NULL;
    }

    // We know LHS is valid, make sure rhs_t == lhs_t
    if (lhs_t->type != rhs_t->type) {
      char *msg = safe_alloc(BUFSIZ);
      sprintf(msg, "Expected type of %s got %s", t_to_str(lhs_t),
              t_to_str(rhs_t));
      typecheck_error(msg, bop_expr->start);
    }

    bop_expr->lhs->t_type = lhs_t;
    bop_expr->rhs->t_type = rhs_t;

    break;
  case UNOPEXPR:;
    unop_expr *uop_expr = (unop_expr *)e->node;
    t *item_t = typeof_expr(uop_expr->rhs, c);
    switch (uop_expr->op) {
    case NEGOP:
      if (item_t->type != FLOAT_T && item_t->type != INT_T) {
        char *msg = safe_alloc(BUFSIZ);
        sprintf(msg, "Expected type of (FloatType) or (IntType) got %s",
                t_to_str(item_t));
        typecheck_error(msg, uop_expr->start);
      }
      break;
    case NOTOP:
      if (item_t->type != BOOL_T) {
        char *msg = safe_alloc(BUFSIZ);
        sprintf(msg, "Expected type of (BoolType) got %s", t_to_str(item_t));
        typecheck_error(msg, uop_expr->start);
      }
      break;
    }

    uop_expr->rhs->t_type = item_t;

    result->type = item_t->type;
    result->info = item_t->info;
    break;
  case STRUCTLITERALEXPR:;
    struct_literal_expr *sle = (struct_literal_expr *)e->node;

    // Check exprs again decleration, and update
    info *sle_exist = check_ctx(c, sle->var);
    if (sle_exist == NULL) {
      char *msg = safe_alloc(BUFSIZ);
      sprintf(msg, "Struct '%s' has not been declared yet", sle->var);
      typecheck_error(msg, sle->start);
    }
    if (sle_exist->type != STRUCTINFO) {
      char *msg = safe_alloc(BUFSIZ);
      sprintf(msg, "Symbol '%s' is not a struct", sle->var);
      typecheck_error(msg, sle->start);
    }

    // check exprs again definition
    struct_info *sle_dec = (struct_info *)sle_exist->node;
    for (int j = 0; j < sle->exprs->size; j++) {
      expr *cur_e = vector_get_expr(sle->exprs, j);
      t *cur_t = typeof_expr(cur_e, c);
      t *dec_t = vector_get_t(sle_dec->ts, j);
      if (cur_t->type != dec_t->type) {
        char *msg = safe_alloc(BUFSIZ);
        sprintf(msg, "Expected type of %s got %s", t_to_str(dec_t),
                t_to_str(cur_t));
        typecheck_error(msg, cur_e->start);
      }

      cur_e->t_type = cur_t;
    }

    result->info = sle_dec;
    result->type = STRUCT_T;
    break;
  case ARRAYLITERALEXPR:;
    array_literal_expr *ale = (array_literal_expr *)e->node;
    t *found_t = NULL;
    for (int i = 0; i < ale->exprs->size; i++) {
      expr *cur_e = vector_get_expr(ale->exprs, i);
      t *cur_t = typeof_expr(cur_e, c);
      if (found_t == NULL) {
        cur_e->t_type = cur_t;
        found_t = cur_t;
      } else {
        if (found_t->type != cur_t->type) {
          char *msg = safe_alloc(BUFSIZ);
          sprintf(msg, "Expected type of %s got %s", t_to_str(found_t),
                  t_to_str(cur_t));
          typecheck_error(msg, cur_e->start);
        } else {
          cur_e->t_type = cur_t;
        }
      }
    }

    array_info *a_info = safe_alloc(sizeof(array_info));
    a_info->type = found_t;
    a_info->rank = 1;

    result->info = a_info;
    result->type = ARRAY_T;
    break;
  case EXPR:;
    t *inner_t = typeof_expr((expr *)e->node, c);
    ((expr *)e->node)->t_type = inner_t;
    free(result);
    result = inner_t;
    break;
  case IFEXPR:;
    if_expr *if_e = (if_expr *)e->node;
    t *if_t = typeof_expr(if_e->if_expr, c);
    t *then_t = typeof_expr(if_e->then_expr, c);
    t *else_t = typeof_expr(if_e->else_expr, c);

    if (if_t->type != BOOL_T) {
      char *msg = safe_alloc(BUFSIZ);
      sprintf(msg, "Expected type of (BoolType) got %s", t_to_str(if_t));
      typecheck_error(msg, if_e->if_expr->start);
    }

    if (then_t->type != else_t->type) {
      char *msg = safe_alloc(BUFSIZ);
      sprintf(msg, "Expected type of %s got %s", t_to_str(then_t),
              t_to_str(else_t));
      typecheck_error(msg, if_e->else_expr->start);
    }

    if_e->if_expr->t_type = if_t;
    if_e->then_expr->t_type = then_t;
    if_e->else_expr->t_type = else_t;

    result->type = then_t->type;
    result->info = then_t->info;
    break;
  case DOTEXPR:;
    dot_expr *de = (dot_expr *)e->node;
    t *de_lhs_t = typeof_expr(de->expr, c);
    if (de_lhs_t->type != STRUCT_T) {
      char *msg = safe_alloc(BUFSIZ);
      sprintf(msg, "Expected type of (StructType) got %s", t_to_str(de_lhs_t));
      typecheck_error(msg, de->start);
    }
    de->expr->t_type = de_lhs_t;

    struct_info *lhs_info = (struct_info *)de_lhs_t->info;
    t *found = NULL;
    for (int i = 0; i < lhs_info->vars->size; i++) {
      if (!strcmp(vector_get_str(lhs_info->vars, i), de->var)) {
        found = vector_get_t(lhs_info->ts, i);
        break;
      }
    }
    if (found == NULL) {
      printf("Var '%s' is not a member of "
             "struct '%s'\n",
             de->var, lhs_info->name);
      exit(EXIT_FAILURE);
    }
    result->type = found->type;
    result->info = found->info;
    break;
  case ARRAYINDEXEXPR:;
    array_index_expr *aie = (array_index_expr *)e->node;
    t *aie_lhs_t = typeof_expr(aie->expr, c);
    if (aie_lhs_t->type != ARRAY_T) {
      char *msg = safe_alloc(BUFSIZ);
      sprintf(msg, "Expected type of (ArrayType) got %s", t_to_str(aie_lhs_t));
      typecheck_error(msg, aie->start);
    }
    aie->expr->t_type = aie_lhs_t;

    array_info *aie_info = (array_info *)aie_lhs_t->info;
    if (aie_info->rank != aie->exprs->size) {
      char *msg = safe_alloc(BUFSIZ);
      sprintf(msg,
              "Expected index of rank %d, but "
              "was of rank %d at %d\n",
              aie_info->rank, aie->exprs->size, aie->start);
      typecheck_error(msg, aie->start);
    }

    for (int i = 0; i < aie->exprs->size; i++) {
      expr *cur_e = vector_get_expr(aie->exprs, i);
      t *cur_t = typeof_expr(cur_e, c);
      if (cur_t->type != INT_T) {
        char *msg = safe_alloc(BUFSIZ);
        sprintf(msg, "Expected type of (IntType) got %s", t_to_str(cur_t));
        typecheck_error(msg, cur_e->start);
      }
      cur_e->t_type = cur_t;
    }

    result->type = aie_info->type->type;
    result->info = aie_info->type->info;
    break;
  case VAREXPR:;
    var_expr *ve = (var_expr *)e->node;
    info *ve_def = check_ctx(c, ve->var);
    if (ve_def == NULL) {
      char *msg = safe_alloc(BUFSIZ);
      sprintf(msg, "Variable '%s' has not been defined yet", ve->var);
      typecheck_error(msg, ve->start);
    }

    switch (ve_def->type) {
    case STRUCTINFO:;
      struct_info *si = (struct_info *)ve_def->node;
      result->type = STRUCT_T;
      result->info = si;
      break;
    case ARRAYINFO:;
      array_info *ai = (array_info *)ve_def->node;
      result->type = ARRAY_T;
      result->info = ai;
      break;
    case VARINFO:;
      var_info *vi = (var_info *)ve_def->node;
      free(result);
      result = vi->t;
      break;
    default:;
      typecheck_error("Expected variable, found function", ve->start);
    }
    break;
  case CALLEXPR:;
    call_expr *ce = (call_expr *)e->node;

    // Check for function in ctx
    info *ce_exist = check_ctx(c, ce->var);
    if (ce_exist == NULL) {
      char *msg = safe_alloc(BUFSIZ);
      sprintf(msg, "Function '%s' has not been defined yet", ce->var);
      typecheck_error(msg, ce->start);
    }
    if (ce_exist->type != FNINFO) {
      typecheck_error("Expected function, found variable", ce->start);
    }
    fn_info *ce_info = (fn_info *)ce_exist->node;

    // Make sure we have the right number of args
    if (ce_info->args->size != ce->exprs->size) {
      char *msg = safe_alloc(BUFSIZ);
      sprintf(msg, "Function '%s' expects %d args, found %d", ce->var,
              ce_info->args->size, ce->exprs->size);
      typecheck_error(msg, ce->start);
    }

    // Loop through args, check types
    for (int i = 0; i < ce_info->args->size; i++) {
      expr *e = vector_get_expr(ce->exprs, i);
      t *et = typeof_expr(e, c);

      binding *b = vector_get_binding(ce_info->args, i);
      t *deft = typeof_type(b->type, c);

      if (!t_eq(et, deft)) {
        char *msg = safe_alloc(BUFSIZ);
        sprintf(msg, "Argument %d should be of type %s, is of type %s", i,
                t_to_str(deft), t_to_str(et));
        typecheck_error(msg, e->start);
      }
      e->t_type = deft;
    }

    result = ce_info->ret;
    break;
  case ARRAYLOOPEXPR:;
    array_loop_expr *aloop = (array_loop_expr *)e->node;

    // Make sure exprs is not empty
    if (aloop->exprs->size == 0) {
      char *msg = safe_alloc(BUFSIZ);
      sprintf(msg, "Expected Expr, found nothing");
      typecheck_error(msg, e->start);
    }

    // Loop through vars, adding to inner scope
    ctx *aloop_ctx = setup_ctx();
    aloop_ctx->parent = c;
    for (int i = 0; i < aloop->vars->size; i++) {
      expr *e = vector_get_expr(aloop->exprs, i);
      t *et = typeof_expr(e, c);
      if (et->type != INT_T) {
        char *msg = safe_alloc(BUFSIZ);
        sprintf(msg, "Expected (IntType), found %s", t_to_str(et));
        typecheck_error(msg, e->start);
      }
      e->t_type = et;

      char *var = vector_get_str(aloop->vars, i);
      var_info *info = safe_alloc(sizeof(var_info));
      info->t = et;
      info->name = var;
      vector_append(aloop_ctx->vars, info);
    }

    // Typecheck expr now that we have scope
    t *aloop_t = typeof_expr(aloop->expr, aloop_ctx);
    aloop->expr->t_type = aloop_t;
    free(aloop_ctx);

    // Build up type
    array_info *aloop_info = safe_alloc(sizeof(array_info));
    aloop_info->type = aloop_t;
    aloop_info->rank = aloop->vars->size;

    result->type = ARRAY_T;
    result->info = aloop_info;
    break;
  case SUMLOOPEXPR:;
    sum_loop_expr *sloop = (sum_loop_expr *)e->node;

    // Make sure exprs is not empty
    if (sloop->exprs->size == 0) {
      char *msg = safe_alloc(BUFSIZ);
      sprintf(msg, "Expected Expr, found nothing");
      typecheck_error(msg, e->start);
    }

    // Loop through vars, adding to inner scope
    ctx *sloop_ctx = setup_ctx();
    sloop_ctx->parent = c;
    for (int i = 0; i < sloop->vars->size; i++) {
      expr *e = vector_get_expr(sloop->exprs, i);
      t *et = typeof_expr(e, c);
      if (et->type != INT_T) {
        char *msg = safe_alloc(BUFSIZ);
        sprintf(msg, "Expected (IntType), found %s", t_to_str(et));
        typecheck_error(msg, e->start);
      }
      e->t_type = et;

      char *var = vector_get_str(sloop->vars, i);
      var_info *info = safe_alloc(sizeof(var_info));
      info->t = et;
      info->name = var;
      vector_append(sloop_ctx->vars, info);
    }

    // Typecheck expr now that we have scope
    t *sloop_t = typeof_expr(sloop->expr, sloop_ctx);
    free(sloop_ctx);
    if (sloop_t->type != INT_T && sloop_t->type != FLOAT_T) {
      char *msg = safe_alloc(BUFSIZ);
      sprintf(msg, "Expected (IntType) or (FloatType), found %s",
              t_to_str(sloop_t));
      typecheck_error(msg, sloop->expr->start);
    }
    sloop->expr->t_type = sloop_t;

    result->info = NULL;
    result->type = sloop_t->type;
    break;
  }
  return result;
}
