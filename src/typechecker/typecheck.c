#include "typecheck.h"
#include "alloc.h"
#include "compiler_error.h"
#include "ctx.h"
#include "t.h"
#include "vector_get.h"
#include <stdio.h>
#include <string.h>

void typecheck(vector *program) {
  // Setup global ctx
  ctx *global = alloc(sizeof(ctx));
  global->structs = alloc(sizeof(vector));
  global->arrays = alloc(sizeof(vector));
  vector_init(global->structs, 8, STRUCTINFOVECTOR);
  vector_init(global->arrays, 8, ARRAYINFOVECTOR);

  struct_info *rgba = alloc(sizeof(struct_info));
  rgba->name = "rgba";
  rgba->ts = alloc(sizeof(vector));
  vector *b_vec = alloc(sizeof(vector));
  vector_init(b_vec, 4, STRVECTOR);
  vector_init(rgba->ts, 4, TVECTOR);

  vector_append(b_vec, "r");
  vector_append(b_vec, "g");
  vector_append(b_vec, "b");
  vector_append(b_vec, "a");

  for (int i = 0; i < 4; i++) {
    t *float_t = alloc(sizeof(t));
    float_t->type = FLOAT_T;
    vector_append(rgba->ts, float_t);
  }
  vector_append(global->structs, rgba);

  // Loop through cmds
  global->parent = NULL;
  for (int i = 0; i < program->size; i++) {
    cmd *cur_cmd = vector_get_cmd(program, i);
    switch (cur_cmd->type) {
    case STRUCTCMD:;
      struct_cmd *s_cmd = (struct_cmd *)cur_cmd->node;

      struct_info *s_info = alloc(sizeof(struct_info));
      s_info->name = s_cmd->var;
      s_info->vars = s_cmd->vars;

      vector *s_ts = alloc(sizeof(vector));
      vector_init(s_ts, s_cmd->types->size, TVECTOR);
      for (int i = 0; i < s_cmd->types->size; i++) {
        t *cur = typeof_type(vector_get_type(s_cmd->types, i), global);
        vector_append(s_ts, cur);
      }
      s_info->ts = s_ts;
      vector_append(global->structs, s_info);
      break;
    case SHOWCMD:;
      show_cmd *sh_cmd = (show_cmd *)cur_cmd->node;
      t *expr_t = typeof_expr(sh_cmd->expr, global);
      sh_cmd->expr->t_type = expr_t;
      break;
    default:
      printf("Compilation Failed [TYPECHECKER]: CMD not yet implemented\n");
      exit(EXIT_FAILURE);
    }
  }

  return;
}

t *typeof_type(type *type, ctx *c) {
  t *result = alloc(sizeof(t));
  switch (type->type) {
  case INTTYPE:
    result->type = INT_T;
    result->info = NULL;
    break;
  case FLOATTYPE:
    result->type = FLOAT_T;
    result->info = NULL;
    break;
  case BOOLTYPE:
    result->type = BOOL_T;
    result->info = NULL;
    break;
  case VOIDTYPE:
    result->type = VOID_T;
    result->info = NULL;
    break;
  case ARRAYTYPE:;
    array_type *a_t = (array_type *)type->node;
    result->type = ARRAY_T;

    array_info *a_info = alloc(sizeof(array_info));
    a_info->type = typeof_type(a_t->type, c);
    a_info->rank = a_t->rank;
    result->info = a_info;
    break;
  case STRUCTTYPE:;
    struct_type *s_t = (struct_type *)type->node;
    struct_info *s_info = alloc(sizeof(struct_info));
    s_info->name = s_t->var;
    if (!vector_contains(c->structs, s_info)) {
      printf("Compilation Failed [TYPECHECKER]: struct of name '%s' is not "
             "declared at %d\n",
             s_t->var, s_t->start);
      exit(EXIT_FAILURE);
    }

    result->type = STRUCT_T;
    result->info = s_info;
    break;
  }
  return result;
}

t *typeof_expr(expr *e, ctx *c) {
  t *result = alloc(sizeof(t));
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

    // check lhs is valid for op
    switch (bop_expr->op) {
    case ADDOP:
    case SUBOP:
    case MULTOP:
    case DIVOP:
    case MODOP:
      if (lhs_t->type != FLOAT_T && lhs_t->type != INT_T) {
        t expected = {INT_T, NULL};
        typecheck_error(&expected, lhs_t, bop_expr->start);
      }
      result->type = lhs_t->type;
      result->info = NULL;
      break;
    case LTOP:
    case LEOP:
    case GTOP:
    case GEOP:
      if (lhs_t->type != FLOAT_T && lhs_t->type != INT_T) {
        t expected = {INT_T, NULL};
        typecheck_error(&expected, lhs_t, bop_expr->start);
      }
      result->type = BOOL_T;
      result->info = NULL;
      break;
    case ANDOP:
    case OROP:
      if (lhs_t->type != BOOL_T) {
        t expected = {INT_T, NULL};
        typecheck_error(&expected, lhs_t, bop_expr->start);
      }
      break;
      result->type = BOOL_T;
      result->info = NULL;
    case EQOP:
    case NEOP:
      if (lhs_t->type != FLOAT_T && lhs_t->type != INT_T &&
          lhs_t->type != BOOL_T) {
        t expected = {INT_T, NULL};
        typecheck_error(&expected, lhs_t, bop_expr->start);
      }
      result->type = BOOL_T;
      result->info = NULL;
    }

    // we know lhs is valid, make sure rhs_t == lhs_t
    if (lhs_t->type != rhs_t->type)
      typecheck_error(lhs_t, rhs_t, bop_expr->start);

    bop_expr->lhs->t_type = lhs_t;
    bop_expr->rhs->t_type = rhs_t;

    break;
  case UNOPEXPR:;
    unop_expr *uop_expr = (unop_expr *)e->node;
    t *item_t = typeof_expr(uop_expr->rhs, c);
    switch (uop_expr->op) {
    case NEGOP:
      if (item_t->type != FLOAT_T && item_t->type != INT_T) {
        t expected = {INT_T, NULL};
        typecheck_error(&expected, item_t, uop_expr->start);
      }
      break;
    case NOTOP:
      if (item_t->type != BOOL_T) {
        t expected = {INT_T, NULL};
        typecheck_error(&expected, item_t, uop_expr->start);
      }
      break;
    }

    uop_expr->rhs->t_type = item_t;

    result->type = item_t->type;
    result->info = item_t->info;
    break;
  case STRUCTLITERALEXPR:;
    struct_literal_expr *sle = (struct_literal_expr *)e->node;

    // check exprs again decleration, and update
    struct_info *dec = NULL;
    for (int i = 0; i < c->structs->size; i++) {
      dec = vector_get_struct_info(c->structs, i);
      if (strcmp(dec->name, sle->var)) {
        if (i == c->structs->size - 1) {
          printf(
              "Compilation Failed [TYPECHECKER]: struct of name '%s' has not "
              "been declared at %d\n",
              sle->var, sle->start);
        }
        continue;
      }

      for (int j = 0; j < sle->exprs->size; j++) {
        expr *cur_e = vector_get_expr(sle->exprs, j);
        t *cur_t = typeof_expr(cur_e, c);
        t *dec_t = vector_get_t(dec->ts, j);
        if (cur_t->type != dec_t->type)
          typecheck_error(dec_t, cur_t, cur_e->start);

        cur_e->t_type = cur_t;
      }
      break;
    }

    result->type = STRUCT_T;
    result->info = dec;
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
          typecheck_error(found_t, cur_t, cur_e->start);
        } else {
          cur_e->t_type = cur_t;
        }
      }
    }

    array_info *a_info = alloc(sizeof(array_info));
    a_info->type = found_t;
    a_info->rank = ale->exprs->size - 1;

    result->info = a_info;
    result->type = ARRAY_T;
    break;
  case EXPR:;
    t *inner_t = typeof_expr((expr *)e->node, c);
    free(result);
    result = inner_t;
  case IFEXPR:;
    if_expr *if_e = (if_expr *)e->node;
    t *if_t = typeof_expr(if_e->if_expr, c);
    t *then_t = typeof_expr(if_e->then_expr, c);
    t *else_t = typeof_expr(if_e->else_expr, c);

    if (if_t->type != BOOL_T) {
      t expected = {BOOL_T, NULL};
      typecheck_error(&expected, if_t, if_e->if_expr->start);
    }

    if (then_t->type != else_t->type) {
      typecheck_error(then_t, else_t, if_e->else_expr->start);
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
      t expected = {STRUCT_T, NULL};
      typecheck_error(&expected, de_lhs_t, de->start);
    }
    de->expr->t_type = de_lhs_t;

    struct_info *de_s_info = (struct_info *)de_lhs_t->info;
    for (int i = 0; i < de_s_info->vars->size; i++) {
      char *cur_var = vector_get_str(de_s_info->vars, i);
      if (!strcmp(cur_var, de->var)) {
        t *var_t = vector_get_t(de_s_info->ts, i);
        result->type = var_t->type;
        result->info = var_t->info;
        break;
      }
    }

    if (result->info == NULL) {
      printf("Compilation Failed [TYPECHECKER]: variable '%s' is not a member "
             "of struct '%s' at %d\n",
             de->var, de_s_info->name, de->start);
      exit(EXIT_FAILURE);
    }
    break;
  case ARRAYINDEXEXPR:;
    array_index_expr *aie = (array_index_expr *)e->node;
    t *aie_lhs_t = typeof_expr(aie->expr, c);
    if (aie_lhs_t->type != ARRAY_T) {
      t expected = {ARRAY_T, NULL};
      typecheck_error(&expected, aie_lhs_t, aie->start);
    }
    aie->expr->t_type = aie_lhs_t;

    array_info *aie_info = (array_info *)aie_lhs_t->info;
    if (aie_info->rank != aie->exprs->size - 1) {
      printf("Compilation Failed [TYPECHECKER]: expected index of rank %d, but "
             "was of rank %lu at %d\n",
             aie_info->rank, aie->exprs->size, aie->start);
      exit(EXIT_FAILURE);
    }

    for (int i = 0; i < aie->exprs->size; i++) {
      expr *cur_e = vector_get_expr(aie->exprs, i);
      t *cur_t = typeof_expr(cur_e, c);
      if (cur_t->type != INT_T) {
        t expected = {INT_T, NULL};
        typecheck_error(&expected, cur_t, cur_e->start);
      }
      cur_e->t_type = cur_t;
    }

    result->type = aie_info->type->type;
    result->info = aie_info->type->info;
    break;
  default:
    printf("Compilation Failed [TYPECHECKER]: EXPR not yet implemented\n");
    exit(EXIT_FAILURE);
  }
  return result;
}
