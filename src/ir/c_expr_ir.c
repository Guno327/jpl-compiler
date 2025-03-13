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
    result = jpl_to_c(fn, ve->var);
    if (result == NULL)
      result = ve->var;
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
    char *bop_sym = NULL;
    if (bop->op == ANDOP || bop->op == OROP)
      bop_sym = gensym(fn);

    char *bop_lhs = expr_gencode(prog, fn, bop->lhs);
    char *bop_code = safe_alloc(1);

    char *bop_type;
    if (bop->op == ADDOP || bop->op == SUBOP || bop->op == DIVOP ||
        bop->op == MULTOP || bop->op == MODOP) {
      bop_type = gent(prog, fn, bop->rhs->t_type);
    } else {
      bop_type = safe_alloc(5);
      memcpy(bop_type, "bool", 4);
    }

    char *short_circuit_jmp = NULL;
    char *bop_rhs = NULL;

    // Short circuit logic
    if (bop->op == ANDOP || bop->op == OROP) {
      bop_code = safe_strcat(bop_code, bop_type);
      free(bop_type);
      bop_code = safe_strcat(bop_code, " ");
      bop_code = safe_strcat(bop_code, bop_sym);
      bop_code = safe_strcat(bop_code, " = ");
      bop_code = safe_strcat(bop_code, bop_lhs);
      bop_code = safe_strcat(bop_code, ";\n");

      if (bop->op == ANDOP)
        bop_code = safe_strcat(bop_code, "if (0 == ");
      else
        bop_code = safe_strcat(bop_code, "if (0 != ");
      bop_code = safe_strcat(bop_code, bop_lhs);
      bop_code = safe_strcat(bop_code, ")\n");

      short_circuit_jmp = genjmp(prog);
      bop_code = safe_strcat(bop_code, "goto ");
      bop_code = safe_strcat(bop_code, short_circuit_jmp);
      bop_code = safe_strcat(bop_code, ";\n");

      vector_append(fn->code, bop_code);
      bop_code = safe_alloc(1);
      bop_rhs = expr_gencode(prog, fn, bop->rhs);
    } else {
      bop_rhs = expr_gencode(prog, fn, bop->rhs);
      bop_sym = gensym(fn);
    }
    if (bop->op != OROP && bop->op != ANDOP) {
      bop_code = safe_strcat(bop_code, bop_type);
      free(bop_type);
    }
    bop_code = safe_strcat(bop_code, " ");
    bop_code = safe_strcat(bop_code, bop_sym);
    bop_code = safe_strcat(bop_code, " = ");
    if (bop->op != MODOP || bop->lhs->t_type->type != FLOAT_T) {
      switch (bop->op) {
      case ADDOP:
        bop_code = safe_strcat(bop_code, bop_lhs);
        bop_code = safe_strcat(bop_code, " + ");
        bop_code = safe_strcat(bop_code, bop_rhs);
        bop_code = safe_strcat(bop_code, ";\n");
        break;
      case SUBOP:
        bop_code = safe_strcat(bop_code, bop_lhs);
        bop_code = safe_strcat(bop_code, " - ");
        bop_code = safe_strcat(bop_code, bop_rhs);
        bop_code = safe_strcat(bop_code, ";\n");
        break;
      case MULTOP:
        bop_code = safe_strcat(bop_code, bop_lhs);
        bop_code = safe_strcat(bop_code, " * ");
        bop_code = safe_strcat(bop_code, bop_rhs);
        bop_code = safe_strcat(bop_code, ";\n");
        break;
      case DIVOP:
        bop_code = safe_strcat(bop_code, bop_lhs);
        bop_code = safe_strcat(bop_code, " / ");
        bop_code = safe_strcat(bop_code, bop_rhs);
        bop_code = safe_strcat(bop_code, ";\n");
        break;
      case GEOP:
        bop_code = safe_strcat(bop_code, bop_lhs);
        bop_code = safe_strcat(bop_code, " >= ");
        bop_code = safe_strcat(bop_code, bop_rhs);
        bop_code = safe_strcat(bop_code, ";\n");
        break;
      case LEOP:
        bop_code = safe_strcat(bop_code, bop_lhs);
        bop_code = safe_strcat(bop_code, " <= ");
        bop_code = safe_strcat(bop_code, bop_rhs);
        bop_code = safe_strcat(bop_code, ";\n");
        break;
      case GTOP:
        bop_code = safe_strcat(bop_code, bop_lhs);
        bop_code = safe_strcat(bop_code, " > ");
        bop_code = safe_strcat(bop_code, bop_rhs);
        bop_code = safe_strcat(bop_code, ";\n");
        break;
      case LTOP:
        bop_code = safe_strcat(bop_code, bop_lhs);
        bop_code = safe_strcat(bop_code, " < ");
        bop_code = safe_strcat(bop_code, bop_rhs);
        bop_code = safe_strcat(bop_code, ";\n");
        break;
      case EQOP:
        bop_code = safe_strcat(bop_code, bop_lhs);
        bop_code = safe_strcat(bop_code, " == ");
        bop_code = safe_strcat(bop_code, bop_rhs);
        bop_code = safe_strcat(bop_code, ";\n");
        break;
      case NEOP:
        bop_code = safe_strcat(bop_code, bop_lhs);
        bop_code = safe_strcat(bop_code, " != ");
        bop_code = safe_strcat(bop_code, bop_rhs);
        bop_code = safe_strcat(bop_code, ";\n");
        break;
      case ANDOP:
        bop_code = safe_strcat(bop_code, bop_rhs);
        bop_code = safe_strcat(bop_code, ";\n");

        bop_code = safe_strcat(bop_code, short_circuit_jmp);
        free(short_circuit_jmp);
        bop_code = safe_strcat(bop_code, ":;\n");
        break;
      case OROP:
        bop_code = safe_strcat(bop_code, bop_rhs);
        bop_code = safe_strcat(bop_code, ";\n");

        bop_code = safe_strcat(bop_code, short_circuit_jmp);
        free(short_circuit_jmp);
        bop_code = safe_strcat(bop_code, ":;\n");
        break;
      case MODOP:
        bop_code = safe_strcat(bop_code, bop_lhs);
        bop_code = safe_strcat(bop_code, " % ");
        bop_code = safe_strcat(bop_code, bop_rhs);
        bop_code = safe_strcat(bop_code, ";\n");
        break;
      }
    } else {
      bop_code = safe_strcat(bop_code, "fmod(");
      bop_code = safe_strcat(bop_code, bop_lhs);
      bop_code = safe_strcat(bop_code, ", ");
      bop_code = safe_strcat(bop_code, bop_rhs);
      bop_code = safe_strcat(bop_code, ")");
      bop_code = safe_strcat(bop_code, ";\n");
    }
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
  case SUMLOOPEXPR:;
    sum_loop_expr *sloop = (sum_loop_expr *)e->node;
    char *sloop_sym = gensym(fn);
    char *sloop_type = gent(prog, fn, e->t_type);
    char *sloop_code = safe_alloc(1);

    // Bind out (and save for ordering)
    sloop_code = safe_strcat(sloop_code, sloop_type);
    free(sloop_type);
    sloop_code = safe_strcat(sloop_code, " ");
    sloop_code = safe_strcat(sloop_code, sloop_sym);
    sloop_code = safe_strcat(sloop_code, ";\n");
    vector_append(fn->code, sloop_code);
    sloop_code = safe_alloc(1);

    // Check bounds
    vector *sloop_e_syms = safe_alloc(sizeof(vector));
    vector_init(sloop_e_syms, sloop->exprs->size, STRVECTOR);
    for (int i = 0; i < sloop->exprs->size; i++) {
      expr *cur_expr = vector_get_expr(sloop->exprs, i);
      char *cur_sym = expr_gencode(prog, fn, cur_expr);
      vector_append(sloop_e_syms, cur_sym);

      sloop_code = safe_strcat(sloop_code, "if (");
      sloop_code = safe_strcat(sloop_code, cur_sym);
      sloop_code = safe_strcat(sloop_code, " > 0)\n");

      char *cur_jmp = genjmp(prog);
      sloop_code = safe_strcat(sloop_code, "goto ");
      sloop_code = safe_strcat(sloop_code, cur_jmp);
      sloop_code = safe_strcat(sloop_code, ";\n");

      sloop_code = safe_strcat(
          sloop_code, "fail_assertion(\"non-positive loop bound\");\n");
      sloop_code = safe_strcat(sloop_code, cur_jmp);
      free(cur_jmp);
      sloop_code = safe_strcat(sloop_code, ":;\n");

      vector_append(fn->code, sloop_code);
      sloop_code = safe_alloc(1);
    }

    // Init out
    sloop_code = safe_strcat(sloop_code, sloop_sym);
    sloop_code = safe_strcat(sloop_code, " = 0;\n");

    // Setup fn call for body
    c_fn *sloop_body_fn = safe_alloc(sizeof(c_fn));
    sloop_body_fn->code = fn->code;
    sloop_body_fn->parent = fn;
    sloop_body_fn->c_names = safe_alloc(sizeof(vector));
    sloop_body_fn->jpl_names = safe_alloc(sizeof(vector));
    vector_init(sloop_body_fn->c_names, sloop->exprs->size, STRVECTOR);
    vector_init(sloop_body_fn->jpl_names, sloop->exprs->size, STRVECTOR);

    // Init bounds
    vector *sloop_bounds = safe_alloc(sizeof(vector));
    vector_init(sloop_bounds, sloop_e_syms->size, STRVECTOR);
    for (int i = sloop_e_syms->size - 1; i >= 0; i--) {
      char *i_sym = gensym(fn);
      sloop_code = safe_strcat(sloop_code, "int64_t ");
      sloop_code = safe_strcat(sloop_code, i_sym);
      sloop_code = safe_strcat(sloop_code, " = 0;\n");
      vector_append(sloop_bounds, i_sym);

      char *jpl_name = vector_get_str(sloop->vars, i);
      vector_append(sloop_body_fn->jpl_names, jpl_name);
      vector_append(sloop_body_fn->c_names, i_sym);
    }

    // Loop jmp
    char *sloop_jmp = genjmp(prog);
    sloop_code = safe_strcat(sloop_code, sloop_jmp);
    sloop_code = safe_strcat(sloop_code, ":;\n");
    vector_append(fn->code, sloop_code);
    sloop_code = safe_alloc(1);

    // Loop body
    sloop_body_fn->name_ctr = fn->name_ctr;
    char *sloop_body_sym = expr_gencode(prog, sloop_body_fn, sloop->expr);
    fn->name_ctr = sloop_body_fn->name_ctr;

    sloop_code = safe_strcat(sloop_code, sloop_sym);
    sloop_code = safe_strcat(sloop_code, " += ");
    sloop_code = safe_strcat(sloop_code, sloop_body_sym);
    sloop_code = safe_strcat(sloop_code, ";\n");

    // Increment and check bounds
    for (int i = 0; i < sloop_bounds->size; i++) {
      char *i_limit = vector_get_str(sloop_e_syms, sloop_bounds->size - i - 1);
      char *i_sym = vector_get_str(sloop_bounds, i);
      sloop_code = safe_strcat(sloop_code, i_sym);
      sloop_code = safe_strcat(sloop_code, "++;\n");

      sloop_code = safe_strcat(sloop_code, "if (");
      sloop_code = safe_strcat(sloop_code, i_sym);
      sloop_code = safe_strcat(sloop_code, " < ");
      sloop_code = safe_strcat(sloop_code, i_limit);
      sloop_code = safe_strcat(sloop_code, ")\n");

      sloop_code = safe_strcat(sloop_code, "goto ");
      sloop_code = safe_strcat(sloop_code, sloop_jmp);
      sloop_code = safe_strcat(sloop_code, ";\n");

      if (i != sloop_bounds->size - 1) {
        sloop_code = safe_strcat(sloop_code, i_sym);
        sloop_code = safe_strcat(sloop_code, " = 0;\n");
      }
    }
    free(sloop_bounds);
    free(sloop_e_syms);
    free(sloop_jmp);
    free(sloop_body_fn);

    vector_append(fn->code, sloop_code);
    result = sloop_sym;
    break;
  case ARRAYLOOPEXPR:;
    array_loop_expr *aloop = (array_loop_expr *)e->node;
    char *aloop_sym = gensym(fn);
    char *aloop_type = gent(prog, fn, e->t_type);
    char *aloop_code = safe_alloc(1);

    // Bind out (and save for ordering)
    aloop_code = safe_strcat(aloop_code, aloop_type);
    free(aloop_type);
    aloop_code = safe_strcat(aloop_code, " ");
    aloop_code = safe_strcat(aloop_code, aloop_sym);
    aloop_code = safe_strcat(aloop_code, ";\n");
    vector_append(fn->code, aloop_code);
    aloop_code = safe_alloc(1);

    // Check bounds
    vector *aloop_e_syms = safe_alloc(sizeof(vector));
    vector_init(aloop_e_syms, aloop->exprs->size, STRVECTOR);
    for (int i = 0; i < aloop->exprs->size; i++) {
      // Gen bound expr
      expr *cur_expr = vector_get_expr(aloop->exprs, i);
      char *cur_sym = expr_gencode(prog, fn, cur_expr);
      vector_append(aloop_e_syms, cur_sym);

      // Bind dim
      char *bind_dim = safe_alloc(BUFSIZ);
      sprintf(bind_dim, "%s.d%d", aloop_sym, i);
      aloop_code = safe_strcat(aloop_code, bind_dim);
      free(bind_dim);
      aloop_code = safe_strcat(aloop_code, " = ");
      aloop_code = safe_strcat(aloop_code, cur_sym);
      aloop_code = safe_strcat(aloop_code, ";\n");

      // Check bound
      aloop_code = safe_strcat(aloop_code, "if (");
      aloop_code = safe_strcat(aloop_code, cur_sym);
      aloop_code = safe_strcat(aloop_code, " > 0)\n");

      char *cur_jmp = genjmp(prog);
      aloop_code = safe_strcat(aloop_code, "goto ");
      aloop_code = safe_strcat(aloop_code, cur_jmp);
      aloop_code = safe_strcat(aloop_code, ";\n");

      aloop_code = safe_strcat(
          aloop_code, "fail_assertion(\"non-positive loop bound\");\n");
      aloop_code = safe_strcat(aloop_code, cur_jmp);
      free(cur_jmp);
      aloop_code = safe_strcat(aloop_code, ":;\n");

      vector_append(fn->code, aloop_code);
      aloop_code = safe_alloc(1);
    }

    // Init out
    char *dim_size_sym = gensym(fn);
    aloop_code = safe_strcat(aloop_code, "int64_t ");
    aloop_code = safe_strcat(aloop_code, dim_size_sym);
    aloop_code = safe_strcat(aloop_code, " = 1;\n");

    for (int i = 0; i < aloop->exprs->size; i++) {
      char *cur_dim = vector_get_str(aloop_e_syms, i);
      aloop_code = safe_strcat(aloop_code, dim_size_sym);
      aloop_code = safe_strcat(aloop_code, " *= ");
      aloop_code = safe_strcat(aloop_code, cur_dim);
      aloop_code = safe_strcat(aloop_code, ";\n");
    }

    char *inner_type = gent(prog, fn, aloop->expr->t_type);
    aloop_code = safe_strcat(aloop_code, dim_size_sym);
    aloop_code = safe_strcat(aloop_code, " *= ");
    aloop_code = safe_strcat(aloop_code, "sizeof(");
    aloop_code = safe_strcat(aloop_code, inner_type);
    free(inner_type);
    aloop_code = safe_strcat(aloop_code, ");\n");

    aloop_code = safe_strcat(aloop_code, aloop_sym);
    aloop_code = safe_strcat(aloop_code, ".data = jpl_alloc(");
    aloop_code = safe_strcat(aloop_code, dim_size_sym);
    aloop_code = safe_strcat(aloop_code, ");\n");

    // Setup fn call for body
    c_fn *aloop_body_fn = safe_alloc(sizeof(c_fn));
    aloop_body_fn->code = fn->code;
    aloop_body_fn->parent = fn;
    aloop_body_fn->c_names = safe_alloc(sizeof(vector));
    aloop_body_fn->jpl_names = safe_alloc(sizeof(vector));
    vector_init(aloop_body_fn->c_names, aloop->exprs->size, STRVECTOR);
    vector_init(aloop_body_fn->jpl_names, aloop->exprs->size, STRVECTOR);

    // Init bounds
    vector *aloop_bounds = safe_alloc(sizeof(vector));
    vector_init(aloop_bounds, aloop_e_syms->size, STRVECTOR);
    for (int i = aloop_e_syms->size - 1; i >= 0; i--) {
      char *i_sym = gensym(fn);
      aloop_code = safe_strcat(aloop_code, "int64_t ");
      aloop_code = safe_strcat(aloop_code, i_sym);
      aloop_code = safe_strcat(aloop_code, " = 0;\n");
      vector_append(aloop_bounds, i_sym);

      char *jpl_name = vector_get_str(aloop->vars, i);
      vector_append(aloop_body_fn->jpl_names, jpl_name);
      vector_append(aloop_body_fn->c_names, i_sym);
    }

    // Loop jmp
    char *aloop_jmp = genjmp(prog);
    aloop_code = safe_strcat(aloop_code, aloop_jmp);
    aloop_code = safe_strcat(aloop_code, ":;\n");
    vector_append(fn->code, aloop_code);
    aloop_code = safe_alloc(1);

    // Loop body
    aloop_body_fn->name_ctr = fn->name_ctr;
    char *aloop_body_sym = expr_gencode(prog, aloop_body_fn, aloop->expr);
    fn->name_ctr = aloop_body_fn->name_ctr;

    aloop_code = safe_strcat(aloop_code, "int64_t ");
    char *aloop_idx_sym = gensym(fn);
    aloop_code = safe_strcat(aloop_code, aloop_idx_sym);
    aloop_code = safe_strcat(aloop_code, " = 0;\n");

    for (int i = 0; i < aloop->exprs->size; i++) {
      char *cur_bound =
          vector_get_str(aloop_bounds, aloop_bounds->size - i - 1);

      aloop_code = safe_strcat(aloop_code, aloop_idx_sym);
      aloop_code = safe_strcat(aloop_code, " *= ");
      aloop_code = safe_strcat(aloop_code, aloop_sym);

      aloop_code = safe_strcat(aloop_code, ".d");
      char *dIDX = safe_alloc(BUFSIZ);
      sprintf(dIDX, "%d", i);
      aloop_code = safe_strcat(aloop_code, dIDX);
      free(dIDX);
      aloop_code = safe_strcat(aloop_code, ";\n");

      aloop_code = safe_strcat(aloop_code, aloop_idx_sym);
      aloop_code = safe_strcat(aloop_code, " += ");
      aloop_code = safe_strcat(aloop_code, cur_bound);
      aloop_code = safe_strcat(aloop_code, ";\n");
    }

    aloop_code = safe_strcat(aloop_code, aloop_sym);
    aloop_code = safe_strcat(aloop_code, ".data[");
    aloop_code = safe_strcat(aloop_code, aloop_idx_sym);
    aloop_code = safe_strcat(aloop_code, "] = ");
    aloop_code = safe_strcat(aloop_code, aloop_body_sym);
    aloop_code = safe_strcat(aloop_code, ";\n");

    // Increment and check bounds
    for (int i = 0; i < aloop_bounds->size; i++) {
      char *i_limit = vector_get_str(aloop_e_syms, aloop_bounds->size - i - 1);
      char *i_sym = vector_get_str(aloop_bounds, i);
      aloop_code = safe_strcat(aloop_code, i_sym);
      aloop_code = safe_strcat(aloop_code, "++;\n");

      aloop_code = safe_strcat(aloop_code, "if (");
      aloop_code = safe_strcat(aloop_code, i_sym);
      aloop_code = safe_strcat(aloop_code, " < ");
      aloop_code = safe_strcat(aloop_code, i_limit);
      aloop_code = safe_strcat(aloop_code, ")\n");

      aloop_code = safe_strcat(aloop_code, "goto ");
      aloop_code = safe_strcat(aloop_code, aloop_jmp);
      aloop_code = safe_strcat(aloop_code, ";\n");

      if (i != aloop_bounds->size - 1) {
        aloop_code = safe_strcat(aloop_code, i_sym);
        aloop_code = safe_strcat(aloop_code, " = 0;\n");
      }
    }
    free(aloop_bounds);
    free(aloop_e_syms);
    free(aloop_jmp);
    free(aloop_body_fn);

    vector_append(fn->code, aloop_code);
    result = aloop_sym;
    break;
  default:;
    char *msg = safe_alloc(BUFSIZ);
    sprintf(msg, "expr not yet implemented");
    ir_error(msg);
  }
  return result;
}
