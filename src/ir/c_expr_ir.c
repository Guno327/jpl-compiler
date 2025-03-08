#include "c_ir.h"
#include "compiler_error.h"
#include "safe.h"
#include "typecheck.h"
#include "vector.h"
#include "vector_get.h"
#include <stdio.h>
#include <string.h>

char *expr_gencode(c_prog *prog, c_fn *fn, expr *e) {
  char *result = NULL;
  switch (e->type) {
  case INTEXPR:;
    int_expr *ie = (int_expr *)e->node;
    char *ie_sym = gensym(fn);
    char *ie_val = safe_alloc(BUFSIZ);
    sprintf(ie_val, "%lu", ie->val);

    char *ie_code = safe_alloc(1);
    ie_code = safe_strcat(ie_code, "int64_t ");
    ie_code = safe_strcat(ie_code, ie_sym);
    ie_code = safe_strcat(ie_code, " = ");
    ie_code = safe_strcat(ie_code, ie_val);
    free(ie_val);
    ie_code = safe_strcat(ie_code, ";\n");
    vector_append(fn->code, ie_code);

    vector_append(fn->c_names, ie_sym);
    vector_append(fn->jpl_names, ie_sym);
    result = ie_sym;
    break;
  case FLOATEXPR:;
    float_expr *fe = (float_expr *)e->node;
    char *fe_sym = gensym(fn);
    char *fe_val = safe_alloc(BUFSIZ);
    sprintf(fe_val, "%lu", (long unsigned)fe->val);
    fe_val = safe_strcat(fe_val, ".0");

    char *fe_code = safe_alloc(1);
    fe_code = safe_strcat(fe_code, "double ");
    fe_code = safe_strcat(fe_code, fe_sym);
    fe_code = safe_strcat(fe_code, " = ");
    fe_code = safe_strcat(fe_code, fe_val);
    free(fe_val);
    fe_code = safe_strcat(fe_code, ";\n");
    vector_append(fn->code, fe_code);

    vector_append(fn->c_names, fe_sym);
    vector_append(fn->jpl_names, fe_sym);
    result = fe_sym;
    break;
  case TRUEEXPR:;
    char *te_sym = gensym(fn);

    char *te_code = safe_alloc(1);
    te_code = safe_strcat(te_code, "bool ");
    te_code = safe_strcat(te_code, te_sym);
    te_code = safe_strcat(te_code, " = ");
    te_code = safe_strcat(te_code, "true");
    te_code = safe_strcat(te_code, ";\n");
    vector_append(fn->code, te_code);

    vector_append(fn->c_names, te_sym);
    vector_append(fn->jpl_names, te_sym);
    result = te_sym;
    break;
  case FALSEEXPR:;
    char *fae_sym = gensym(fn);

    char *fae_code = safe_alloc(1);
    fae_code = safe_strcat(fae_code, "bool ");
    fae_code = safe_strcat(fae_code, fae_sym);
    fae_code = safe_strcat(fae_code, " = ");
    fae_code = safe_strcat(fae_code, "false");
    fae_code = safe_strcat(fae_code, ";\n");
    vector_append(fn->code, fae_code);

    vector_append(fn->c_names, fae_sym);
    vector_append(fn->jpl_names, fae_sym);
    result = fae_sym;
    break;
  case VOIDEXPR:;
    char *v_sym = gensym(fn);

    char *v_code = safe_alloc(1);
    v_code = safe_strcat(v_code, "void_t ");
    v_code = safe_strcat(v_code, v_sym);
    v_code = safe_strcat(v_code, " = ");
    v_code = safe_strcat(v_code, "{}");
    v_code = safe_strcat(v_code, ";\n");
    vector_append(fn->code, v_code);

    vector_append(fn->c_names, v_sym);
    vector_append(fn->jpl_names, v_sym);
    result = v_sym;
    break;
  case VAREXPR:;
    var_expr *ve = (var_expr *)e->node;
    char *ve_name = NULL;
    for (int i = 0; i < fn->jpl_names->size; i++) {
      char *cur_name = vector_get_str(fn->jpl_names, i);
      if (!strcmp(ve->var, cur_name)) {
        ve_name = vector_get_str(fn->c_names, i);
        break;
      }
    }
    result = ve_name;
    break;
  case EXPR:;
    expr *inner_e = (expr *)e->node;
    result = expr_gencode(prog, fn, inner_e);
    break;
  case ARRAYLITERALEXPR:;
    array_literal_expr *ale = (array_literal_expr *)e->node;
    vector *ale_results = safe_alloc(sizeof(vector));
    vector_init(ale_results, ale->exprs->size, STRVECTOR);
    for (int i = 0; i < ale->exprs->size; i++) {
      expr *cur_e = vector_get_expr(ale->exprs, i);
      vector_append(ale_results, expr_gencode(prog, fn, cur_e));
    }
    char *ale_sym = gensym(fn);

    array_info *a_info = (array_info *)e->t_type->info;
    char *ale_struct = genarray(prog, fn, a_info->type, a_info->rank);

    char *ale_code = safe_alloc(1);
    // declare array
    ale_code = safe_strcat(ale_code, ale_struct);
    ale_code = safe_strcat(ale_code, " ");
    ale_code = safe_strcat(ale_code, ale_sym);
    ale_code = safe_strcat(ale_code, ";\n");

    // init d1
    ale_code = safe_strcat(ale_code, ale_sym);
    ale_code = safe_strcat(ale_code, ".d0 = ");

    char *ale_size = safe_alloc(BUFSIZ);
    sprintf(ale_size, "%d", ale->exprs->size);
    ale_code = safe_strcat(ale_code, ale_size);
    ale_code = safe_strcat(ale_code, ";\n");

    // alloc data
    ale_code = safe_strcat(ale_code, ale_sym);
    ale_code = safe_strcat(ale_code, ".data = jpl_alloc(sizeof(");
    char *ale_type = gent(prog, fn, a_info->type);
    ale_code = safe_strcat(ale_code, ale_type);
    free(ale_type);
    ale_code = safe_strcat(ale_code, ") * ");
    ale_code = safe_strcat(ale_code, ale_size);
    free(ale_size);
    ale_code = safe_strcat(ale_code, ");\n");

    // init data
    for (int i = 0; i < ale->exprs->size; i++) {
      char *cur_result = vector_get_str(ale_results, i);
      ale_code = safe_strcat(ale_code, ale_sym);
      char *elem = safe_alloc(BUFSIZ);
      sprintf(elem, ".data[%d] = ", i);
      ale_code = safe_strcat(ale_code, elem);
      free(elem);
      ale_code = safe_strcat(ale_code, cur_result);
      ale_code = safe_strcat(ale_code, ";\n");
    }

    // return
    vector_append(fn->code, ale_code);
    return ale_sym;
    break;
  case STRUCTLITERALEXPR:;
    struct_literal_expr *sle = (struct_literal_expr *)e->node;
    vector *sle_results = safe_alloc(sizeof(vector));
    vector_init(sle_results, sle->exprs->size, STRVECTOR);
    for (int i = 0; i < sle->exprs->size; i++) {
      expr *cur_e = vector_get_expr(sle->exprs, i);
      char *cur_sym = expr_gencode(prog, fn, cur_e);
      vector_append(sle_results, cur_sym);
    }
    char *sle_sym = gensym(fn);

    struct_info *s_info = (struct_info *)e->t_type->info;

    char *sle_code = safe_alloc(1);
    sle_code = safe_strcat(sle_code, s_info->name);
    sle_code = safe_strcat(sle_code, " ");
    sle_code = safe_strcat(sle_code, sle_sym);
    sle_code = safe_strcat(sle_code, " = { ");
    for (int i = 0; i < sle_results->size; i++) {
      char *cur_name = vector_get_str(sle_results, i);
      sle_code = safe_strcat(sle_code, cur_name);
      if (i != sle_results->size - 1)
        sle_code = safe_strcat(sle_code, ", ");
    }
    sle_code = safe_strcat(sle_code, " };\n");

    vector_append(fn->code, sle_code);
    result = sle_sym;
    break;
  case UNOPEXPR:;
    unop_expr *uop = (unop_expr *)e->node;
    char *uop_rhs = expr_gencode(prog, fn, uop->rhs);
    char *uop_sym = gensym(fn);

    char *uop_code = safe_alloc(1);
    char *uop_type = gent(prog, fn, uop->rhs->t_type);
    uop_code = safe_strcat(uop_code, uop_type);
    free(uop_type);
    uop_code = safe_strcat(uop_code, " ");
    uop_code = safe_strcat(uop_code, uop_sym);
    uop_code = safe_strcat(uop_code, " = ");
    char *uop_op = uop->op == NEGOP ? "-" : "!";
    uop_code = safe_strcat(uop_code, uop_op);
    uop_code = safe_strcat(uop_code, uop_rhs);
    uop_code = safe_strcat(uop_code, ";\n");

    vector_append(fn->code, uop_code);
    return uop_sym;
    break;
  case BINOPEXPR:;
    binop_expr *bop = (binop_expr *)e->node;
    char *bop_lhs = expr_gencode(prog, fn, bop->lhs);
    char *bop_rhs = expr_gencode(prog, fn, bop->rhs);
    char *bop_sym = gensym(fn);

    char *bop_code = safe_alloc(1);
    if (bop->op == ADDOP || bop->op == SUBOP || bop->op == DIVOP ||
        bop->op == MULTOP || bop->op == MODOP) {
      char *bop_type = gent(prog, fn, bop->rhs->t_type);
      bop_code = safe_strcat(bop_code, bop_type);
      free(bop_type);
    } else {
      bop_code = safe_strcat(bop_code, "bool");
    }
    bop_code = safe_strcat(bop_code, " ");
    bop_code = safe_strcat(bop_code, bop_sym);
    bop_code = safe_strcat(bop_code, " = ");
    if (bop->op != MODOP || bop->lhs->t_type->type != FLOAT_T) {
      bop_code = safe_strcat(bop_code, bop_lhs);
      switch (bop->op) {
      case ADDOP:
        bop_code = safe_strcat(bop_code, " + ");
        break;
      case SUBOP:
        bop_code = safe_strcat(bop_code, " - ");
        break;
      case MULTOP:
        bop_code = safe_strcat(bop_code, " * ");
        break;
      case DIVOP:
        bop_code = safe_strcat(bop_code, " / ");
        break;
      case GEOP:
        bop_code = safe_strcat(bop_code, " >= ");
        break;
      case LEOP:
        bop_code = safe_strcat(bop_code, " <= ");
        break;
      case GTOP:
        bop_code = safe_strcat(bop_code, " > ");
        break;
      case LTOP:
        bop_code = safe_strcat(bop_code, " < ");
        break;
      case EQOP:
        bop_code = safe_strcat(bop_code, " == ");
        break;
      case NEOP:
        bop_code = safe_strcat(bop_code, " != ");
        break;
      case ANDOP:
        bop_code = safe_strcat(bop_code, " && ");
        break;
      case OROP:
        bop_code = safe_strcat(bop_code, " || ");
        break;
      case MODOP:
        bop_code = safe_strcat(bop_code, " % ");
        break;
      }
      bop_code = safe_strcat(bop_code, bop_rhs);
    } else {
      bop_code = safe_strcat(bop_code, "fmod(");
      bop_code = safe_strcat(bop_code, bop_lhs);
      bop_code = safe_strcat(bop_code, ", ");
      bop_code = safe_strcat(bop_code, bop_rhs);
      bop_code = safe_strcat(bop_code, ")");
    }
    bop_code = safe_strcat(bop_code, ";\n");
    vector_append(fn->code, bop_code);
    return bop_sym;
    break;
  case DOTEXPR:;
    dot_expr *de = (dot_expr *)e->node;
    char *de_lhs = expr_gencode(prog, fn, de->expr);
    char *de_sym = gensym(fn);

    char *de_code = safe_alloc(BUFSIZ);
    char *de_type = gent(prog, fn, e->t_type);
    de_code = safe_strcat(de_code, de_type);
    free(de_type);
    de_code = safe_strcat(de_code, " ");
    de_code = safe_strcat(de_code, de_sym);
    de_code = safe_strcat(de_code, " = ");
    de_code = safe_strcat(de_code, de_lhs);
    de_code = safe_strcat(de_code, ".");
    de_code = safe_strcat(de_code, de->var);
    de_code = safe_strcat(de_code, ";\n");
    vector_append(fn->code, de_code);
    result = de_sym;
    break;
  case ARRAYINDEXEXPR:;
    array_index_expr *aie = (array_index_expr *)e->node;
    char *aie_lhs = expr_gencode(prog, fn, aie->expr);

    // Index gencode
    vector *idxs = safe_alloc(sizeof(vector));
    vector_init(idxs, aie->exprs->size, STRVECTOR);
    for (int i = 0; i < aie->exprs->size; i++) {
      expr *cur_expr = vector_get_expr(aie->exprs, i);
      char *cur_sym = expr_gencode(prog, fn, cur_expr);
      vector_append(idxs, cur_sym);
    }

    // Bounds checks
    vector *idx_syms = safe_alloc(sizeof(vector));
    vector_init(idx_syms, idxs->size, STRVECTOR);
    char *aie_code = safe_alloc(1);
    for (int i = 0; i < idxs->size; i++) {
      char *cur_idx = vector_get_str(idxs, i);
      vector_append(idx_syms, cur_idx);

      // Negative bounds check
      char *neg_jmp = genjmp(prog);
      aie_code = safe_strcat(aie_code, "if (");
      aie_code = safe_strcat(aie_code, cur_idx);
      aie_code = safe_strcat(aie_code, " >= 0)\n");

      aie_code = safe_strcat(aie_code, "goto ");
      aie_code = safe_strcat(aie_code, neg_jmp);
      aie_code = safe_strcat(aie_code, ";\n");

      aie_code =
          safe_strcat(aie_code, "fail_assertion(\"negative array index\");\n");
      aie_code = safe_strcat(aie_code, neg_jmp);
      free(neg_jmp);
      aie_code = safe_strcat(aie_code, ":;\n");

      // Upper bounds check
      char *up_jmp = genjmp(prog);
      aie_code = safe_strcat(aie_code, "if (");
      aie_code = safe_strcat(aie_code, cur_idx);
      aie_code = safe_strcat(aie_code, " < ");
      aie_code = safe_strcat(aie_code, aie_lhs);
      aie_code = safe_strcat(aie_code, ".d");
      char *dIDX = safe_alloc(BUFSIZ);
      sprintf(dIDX, "%d", i);
      aie_code = safe_strcat(aie_code, dIDX);
      free(dIDX);
      aie_code = safe_strcat(aie_code, ")\n");

      aie_code = safe_strcat(aie_code, "goto ");
      aie_code = safe_strcat(aie_code, up_jmp);
      aie_code = safe_strcat(aie_code, ";\n");

      aie_code =
          safe_strcat(aie_code, "fail_assertion(\"index too large\");\n");
      aie_code = safe_strcat(aie_code, up_jmp);
      free(up_jmp);
      aie_code = safe_strcat(aie_code, ":;\n");
    }

    // Make C index
    aie_code = safe_strcat(aie_code, "int64_t ");
    char *idx_sym = gensym(fn);
    aie_code = safe_strcat(aie_code, idx_sym);
    aie_code = safe_strcat(aie_code, " = 0;\n");

    for (int i = 0; i < idxs->size; i++) {
      char *cur_idx_sym = vector_get_str(idx_syms, i);

      aie_code = safe_strcat(aie_code, idx_sym);
      aie_code = safe_strcat(aie_code, " *= ");
      aie_code = safe_strcat(aie_code, aie_lhs);
      aie_code = safe_strcat(aie_code, ".d");
      char *dIDX = safe_alloc(BUFSIZ);
      sprintf(dIDX, "%d", i);
      aie_code = safe_strcat(aie_code, dIDX);
      free(dIDX);
      aie_code = safe_strcat(aie_code, ";\n");

      aie_code = safe_strcat(aie_code, idx_sym);
      aie_code = safe_strcat(aie_code, " += ");
      aie_code = safe_strcat(aie_code, cur_idx_sym);
      aie_code = safe_strcat(aie_code, ";\n");
    }

    // Index into data
    char *aie_type = gent(prog, fn, e->t_type);
    aie_code = safe_strcat(aie_code, aie_type);
    free(aie_type);
    char *aie_sym = gensym(fn);
    aie_code = safe_strcat(aie_code, " ");
    aie_code = safe_strcat(aie_code, aie_sym);
    aie_code = safe_strcat(aie_code, " = ");
    aie_code = safe_strcat(aie_code, aie_lhs);
    aie_code = safe_strcat(aie_code, ".data[");
    aie_code = safe_strcat(aie_code, idx_sym);
    free(idx_sym);
    aie_code = safe_strcat(aie_code, "];\n");

    vector_append(fn->code, aie_code);
    result = aie_sym;
    break;
  case IFEXPR:;
    if_expr *ife = (if_expr *)e->node;
    char *if_sym = expr_gencode(prog, fn, ife->if_expr);
    char *ife_sym = gensym(fn);
    char *ife_type = gent(prog, fn, ife->then_expr->t_type);

    char *ife_code = safe_alloc(1);
    // OUT decleration
    ife_code = safe_strcat(ife_code, ife_type);
    ife_code = safe_strcat(ife_code, " ");
    ife_code = safe_strcat(ife_code, ife_sym);
    ife_code = safe_strcat(ife_code, ";\n");

    // IF code
    char *else_jmp = genjmp(prog);
    ife_code = safe_strcat(ife_code, "if (!");
    ife_code = safe_strcat(ife_code, if_sym);
    free(if_sym);
    ife_code = safe_strcat(ife_code, ")\n");

    ife_code = safe_strcat(ife_code, "goto ");
    ife_code = safe_strcat(ife_code, else_jmp);
    ife_code = safe_strcat(ife_code, ";\n");

    // THEN code (we have to save here so the code is in right order)
    vector_append(fn->code, ife_code);
    ife_code = safe_alloc(1);
    char *then_sym = expr_gencode(prog, fn, ife->then_expr);

    ife_code = safe_strcat(ife_code, ife_sym);
    ife_code = safe_strcat(ife_code, " = ");
    ife_code = safe_strcat(ife_code, then_sym);
    free(then_sym);
    ife_code = safe_strcat(ife_code, ";\n");

    char *end_jmp = genjmp(prog);
    ife_code = safe_strcat(ife_code, "goto ");
    ife_code = safe_strcat(ife_code, end_jmp);
    ife_code = safe_strcat(ife_code, ";\n");

    // ELSE code
    ife_code = safe_strcat(ife_code, else_jmp);
    ife_code = safe_strcat(ife_code, ":;\n");

    // We have to save here so the code is in the right order
    vector_append(fn->code, ife_code);
    ife_code = safe_alloc(1);
    char *else_sym = expr_gencode(prog, fn, ife->else_expr);

    ife_code = safe_strcat(ife_code, ife_sym);
    ife_code = safe_strcat(ife_code, " = ");
    ife_code = safe_strcat(ife_code, else_sym);
    free(else_sym);
    ife_code = safe_strcat(ife_code, ";\n");

    ife_code = safe_strcat(ife_code, end_jmp);
    ife_code = safe_strcat(ife_code, ":;\n");

    vector_append(fn->code, ife_code);
    result = ife_sym;
    break;
  case CALLEXPR:;
    call_expr *ce = (call_expr *)e->node;
    vector *ce_syms = safe_alloc(sizeof(vector));
    vector_init(ce_syms, ce->exprs->size, STRVECTOR);
    for (int i = 0; i < ce->exprs->size; i++) {
      expr *cur_expr = vector_get_expr(ce->exprs, i);
      char *cur_sym = expr_gencode(prog, fn, cur_expr);
      vector_append(ce_syms, cur_sym);
    }

    char *ce_code = safe_alloc(1);
    char *ce_type = gent(prog, fn, e->t_type);
    ce_code = safe_strcat(ce_code, ce_type);
    free(ce_type);
    ce_code = safe_strcat(ce_code, " ");
    char *ce_sym = gensym(fn);
    ce_code = safe_strcat(ce_code, ce_sym);
    ce_code = safe_strcat(ce_code, " = ");
    ce_code = safe_strcat(ce_code, ce->var);
    ce_code = safe_strcat(ce_code, "(");

    for (int i = 0; i < ce_syms->size; i++) {
      char *cur_sym = vector_get_str(ce_syms, i);
      ce_code = safe_strcat(ce_code, cur_sym);
      if (i != ce_syms->size - 1)
        ce_code = safe_strcat(ce_code, ", ");
    }
    ce_code = safe_strcat(ce_code, ");\n");

    vector_append(fn->code, ce_code);
    result = ce_sym;
    break;
  // HW9
  case ARRAYLOOPEXPR:;
  case SUMLOOPEXPR:;
  default:;
    char *msg = safe_alloc(BUFSIZ);
    sprintf(msg, "expr not yet implemented");
    ir_error(msg);
  }
  return result;
}
