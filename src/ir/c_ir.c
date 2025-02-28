#include "c_ir.h"
#include "compiler_error.h"
#include "safe.h"
#include "vector.h"
#include "vector_get.h"
#include <stdio.h>
#include <string.h>

c_prog *gen_c_ir(vector *cmds, ctx *ctx) {
  // Setup program env
  c_prog *program = safe_alloc(sizeof(c_prog));
  program->ctx = ctx;
  program->jump_ctr = 0;
  program->fns = safe_alloc(sizeof(vector));
  program->structs = safe_alloc(sizeof(c_struct));
  vector_init(program->fns, 8, CFNVECTOR);
  vector_init(program->structs, 8, CSTRUCTVECTOR);

  // Setup jpl_main
  c_fn *jpl_main = safe_alloc(sizeof(c_fn));
  jpl_main->name = safe_alloc(9);
  memcpy(jpl_main->name, "jpl_main", 8);
  jpl_main->parent = program;
  jpl_main->name_ctr = 0;
  jpl_main->code = safe_alloc(sizeof(vector));
  jpl_main->c_names = safe_alloc(sizeof(vector));
  jpl_main->jpl_names = safe_alloc(sizeof(vector));
  vector_init(jpl_main->code, 8, STRVECTOR);
  vector_init(jpl_main->c_names, 8, STRVECTOR);
  vector_init(jpl_main->jpl_names, 8, STRVECTOR);
  vector_append(program->fns, jpl_main);

  // Handle cmds
  for (int i = 0; i < cmds->size; i++) {
    cmd *c = vector_get_cmd(cmds, i);
    switch (c->type) {
    case LETCMD:;
      let_cmd *lc = (let_cmd *)c->node;
      expr_gencode(program, jpl_main, lc->expr);

    case READCMD:
    case WRITECMD:
    case ASSERTCMD:
    case PRINTCMD:
    case TIMECMD:
    case SHOWCMD:
    case FNCMD:
    case STRUCTCMD:;
    }
  }

  return program;
}

char *gensym(c_fn *fn) {
  char *result = safe_alloc(1);
  result = safe_strcat(result, "_");

  char *num = safe_alloc(BUFSIZ);
  sprintf(num, "%d", fn->name_ctr);
  fn->name_ctr += 1;

  result = safe_strcat(result, num);
  free(num);

  return result;
}

char *expr_gencode(c_prog *prog, c_fn *main, expr *e) {
  char *result;
  switch (e->type) {
  case INTEXPR:;
    int_expr *ie = (int_expr *)e->node;
    char *ie_sym = gensym(main);
    char *ie_val = safe_alloc(BUFSIZ);
    sprintf(ie_val, "%lu", ie->val);

    char *ie_code = safe_alloc(1);
    ie_code = safe_strcat(ie_code, "int64_t ");
    ie_code = safe_strcat(ie_code, ie_sym);
    ie_code = safe_strcat(ie_code, " = ");
    ie_code = safe_strcat(ie_code, ie_val);
    free(ie_val);
    ie_code = safe_strcat(ie_code, ";\n");
    vector_append(main->code, ie_code);

    vector_append(main->c_names, ie_sym);
    vector_append(main->jpl_names, ie_sym);
    result = ie_sym;
    break;
  case FLOATEXPR:;
    float_expr *fe = (float_expr *)e->node;
    char *fe_sym = gensym(main);
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
    vector_append(main->code, fe_code);

    vector_append(main->c_names, fe_sym);
    vector_append(main->jpl_names, fe_sym);
    result = fe_sym;
    break;
  case TRUEEXPR:;
    char *te_sym = gensym(main);

    char *te_code = safe_alloc(1);
    te_code = safe_strcat(te_code, "bool ");
    te_code = safe_strcat(te_code, te_sym);
    te_code = safe_strcat(te_code, " = ");
    te_code = safe_strcat(te_code, "true");
    te_code = safe_strcat(te_code, ";\n");
    vector_append(main->code, te_code);

    vector_append(main->c_names, te_sym);
    vector_append(main->jpl_names, te_sym);
    result = te_sym;
    break;
  case FALSEEXPR:;
    char *fae_sym = gensym(main);

    char *fae_code = safe_alloc(1);
    fae_code = safe_strcat(fae_code, "bool ");
    fae_code = safe_strcat(fae_code, fae_sym);
    fae_code = safe_strcat(fae_code, " = ");
    fae_code = safe_strcat(fae_code, "false");
    fae_code = safe_strcat(fae_code, ";\n");
    vector_append(main->code, fae_code);

    vector_append(main->c_names, fae_sym);
    vector_append(main->jpl_names, fae_sym);
    result = fae_sym;
    break;
  case VOIDEXPR:;
    char *v_sym = gensym(main);

    char *v_code = safe_alloc(1);
    v_code = safe_strcat(v_code, "void ");
    v_code = safe_strcat(v_code, v_sym);
    v_code = safe_strcat(v_code, " = ");
    v_code = safe_strcat(v_code, "{}");
    v_code = safe_strcat(v_code, ";\n");
    vector_append(main->code, v_code);

    vector_append(main->c_names, v_sym);
    vector_append(main->jpl_names, v_sym);
    result = v_sym;
    break;
  case VAREXPR:;
    var_expr *ve = (var_expr *)e->node;
    char *ve_name = NULL;
    for (int i = 0; i < main->jpl_names->size; i++) {
      char *cur_name = vector_get_str(main->jpl_names, i);
      if (!strcmp(ve->var, cur_name)) {
        ve_name = vector_get_str(main->c_names, i);
        break;
      }
    }
    result = ve_name;
    break;
  case EXPR:;
    expr *inner_e = (expr *)e->node;
    result = expr_gencode(prog, main, inner_e);
    break;
  case ARRAYLITERALEXPR:;
    array_literal_expr *ale = (array_literal_expr *)e->node;
    vector *ale_results = safe_alloc(sizeof(vector));
    vector_init(ale_results, ale->exprs->size, STRVECTOR);
    for (int i = 0; i < ale->exprs->size; i++) {
      expr *cur_e = vector_get_expr(ale->exprs, i);
      vector_append(ale_results, expr_gencode(prog, main, cur_e));
    }
    char *ale_sym = gensym(main);
    char *ale_struct = genarray(prog, main, e->t_type, 1);

    char *ale_code = safe_alloc(1);
    // declare array
    ale_code = safe_strcat(ale_code, ale_struct);
    ale_code = safe_strcat(ale_code, " ");
    ale_code = safe_strcat(ale_code, ale_sym);
    ale_code = safe_strcat(ale_code, ";\n");

    // init d1
    ale_code = safe_strcat(ale_code, ale_sym);
    ale_code = safe_strcat(ale_code, ".d1 = ");

    char *ale_size = safe_alloc(BUFSIZ);
    sprintf(ale_size, "%d", ale->exprs->size);
    ale_code = safe_strcat(ale_code, ale_size);
    ale_code = safe_strcat(ale_code, ";\n");

    // alloc data
    ale_code = safe_strcat(ale_code, ale_sym);
    ale_code = safe_strcat(ale_code, ".data = jpl_alloc(sizeof(");
    char *ale_type = gentype(e->t_type);
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
    vector_append(main->code, ale_code);
    return ale_sym;
    break;
  case STRUCTLITERALEXPR:;
    struct_literal_expr *sle = (struct_literal_expr *)e->node;
    vector *sle_results = safe_alloc(sizeof(vector));
    vector_init(sle_results, sle->exprs->size, STRVECTOR);
    for (int i = 0; i < sle->exprs->size; i++) {
      expr *cur_e = vector_get_expr(sle->exprs, i);
      vector_append(sle_results, expr_gencode(prog, main, cur_e));
    }
    char *sle_sym = genstruct(prog, main, sle->var, sle_results);

    char *sle_code = safe_alloc(1);
    sle_code = safe_strcat(sle_code, sle_sym);
    sle_code = safe_strcat(sle_code, " = {");
    for (int i = 0; i < sle_results->size; i++) {
      char *cur_name = vector_get_str(sle_results, i);
      sle_code = safe_strcat(sle_code, cur_name);
      if (i != sle_results->size - 1)
        sle_code = safe_strcat(sle_code, ", ");
    }
    sle_code = safe_strcat(sle_code, "};\n");

    vector_append(main->code, sle_code);
    result = sle_sym;
    break;
  case UNOPEXPR:;
    unop_expr *uop = (unop_expr *)e->node;
    char *uop_rhs = expr_gencode(prog, main, uop->rhs);
    char *uop_sym = gensym(main);

    char *uop_code = safe_alloc(1);
    char *uop_type = gentype(uop->rhs->t_type);
    uop_code = safe_strcat(uop_code, uop_type);
    free(uop_type);
    uop_code = safe_strcat(uop_code, " ");
    uop_code = safe_strcat(uop_code, uop_sym);
    uop_code = safe_strcat(uop_code, " = ");
    char *uop_op = uop->op == NEGOP ? "-" : "!";
    uop_code = safe_strcat(uop_code, uop_rhs);
    uop_code = safe_strcat(uop_code, ";\n");

    vector_append(main->code, uop_code);
    return uop_sym;
    break;
  case BINOPEXPR:;
    binop_expr *bop = (binop_expr *)e->node;
    char *bop_lhs = expr_gencode(prog, main, bop->lhs);
    char *bop_rhs = expr_gencode(prog, main, bop->rhs);
    char *bop_sym = gensym(main);

    char *bop_code = safe_alloc(1);
    char *bop_type = gentype(bop->rhs->t_type);
    bop_code = safe_strcat(bop_code, bop_type);
    free(bop_type);
    bop_code = safe_strcat(bop_code, " ");
    bop_code = safe_strcat(bop_code, bop_sym);
    bop_code = safe_strcat(bop_code, " = ");
    if (bop->op != MODOP || bop->lhs->t_type->type != FLOAT_T) {
      bop_code = safe_strcat(bop_code, bop_rhs);
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
      bop_code = safe_strcat(bop_code, bop_lhs);
    } else {
      bop_code = safe_strcat(bop_code, "fmod(");
      bop_code = safe_strcat(bop_code, bop_lhs);
      bop_code = safe_strcat(bop_code, ", ");
      bop_code = safe_strcat(bop_code, bop_rhs);
      bop_code = safe_strcat(bop_code, ")");
    }
    bop_code = safe_strcat(bop_code, ";\n");
    vector_append(main->code, bop_code);
    return bop_sym;
    break;
  case DOTEXPR:;
    dot_expr *de = (dot_expr *)e->node;
    char *de_lhs = expr_gencode(prog, main, de->expr);
    char *de_sym = gensym(main);

    char *de_code = safe_alloc(BUFSIZ);
    char *de_type = gentype(de->expr->t_type);
    de_code = safe_strcat(de_code, de_type);
    free(de_type);
    de_code = safe_strcat(de_code, " ");
    de_code = safe_strcat(de_code, de_sym);
    de_code = safe_strcat(de_code, " = ");
    de_code = safe_strcat(de_code, de_lhs);
    de_code = safe_strcat(de_code, ".");
    de_code = safe_strcat(de_code, de->var);
    de_code = safe_strcat(de_code, ";\n");
    vector_append(main->code, de_code);
    result = de_sym;
    break;
  case ARRAYINDEXEXPR:;
    array_index_expr *aie = (array_index_expr *)e->node;
  case CALLEXPR:;
    call_expr *ce = (call_expr *)e->node;
  case IFEXPR:;
    if_expr *ife = (if_expr *)e->node;
  case ARRAYLOOPEXPR:;
    array_loop_expr *aloop = (array_loop_expr *)e->node;
  case SUMLOOPEXPR:;
    sum_loop_expr *sloop = (sum_loop_expr *)e->node;
  }
  return result;
}
