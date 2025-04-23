#include "asm_ir.h"
#include "compiler_error.h"
#include "safe.h"
#include "vector_get.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

void expr_asmgen(asm_prog *prog, asm_fn *fn, expr *e) {
  switch (e->type) {
  case EXPR:
    expr_asmgen(prog, fn, e->node);
    break;
  case FLOATEXPR:;
    float_expr *fe = (float_expr *)e->node;
    char *fe_val = safe_alloc(BUFSIZ);
    sprintf(fe_val, "dq %s", fe->val_str);
    char *fe_const = genconst(prog, fe_val);

    char *fe_code = safe_alloc(1);
    fe_code = safe_strcat(fe_code, "mov rax, [rel ");
    fe_code = safe_strcat(fe_code, fe_const);
    fe_code = safe_strcat(fe_code, "]\n");
    vector_append(fn->code, fe_code);

    stack_push(fn, "rax");
    stack_rechar(fn, e->t_type, 1);
    break;
  case INTEXPR:;
    int_expr *ie = (int_expr *)e->node;
    char *ie_val = safe_alloc(BUFSIZ);
    sprintf(ie_val, "dq %ld", ie->val);

    char *ie_code = NULL;
    if (opt > 0 && ie->val <= INT_MAX && ie->val >= INT_MIN) {
      ie_code = safe_alloc(BUFSIZ);
      sprintf(ie_code, "qword %d", (int)ie->val);
      stack_push(fn, ie_code);
      free(ie_code);
    } else {
      char *ie_const = genconst(prog, ie_val);

      ie_code = safe_alloc(1);
      ie_code = safe_strcat(ie_code, "mov rax, [rel ");
      ie_code = safe_strcat(ie_code, ie_const);
      ie_code = safe_strcat(ie_code, "]\n");
      vector_append(fn->code, ie_code);

      stack_push(fn, "rax");
      stack_rechar(fn, e->t_type, 1);
    }
    break;
  case TRUEEXPR:
  case FALSEEXPR:;
    char *be_val = safe_alloc(BUFSIZ);
    long be_num = e->type == TRUEEXPR ? 1 : 0;
    if (opt > 0) {
      char *be_code = safe_alloc(BUFSIZ);
      sprintf(be_code, "qword %d", (int)be_num);
      stack_push(fn, be_code);
      stack_rechar(fn, e->t_type, 1);
      free(be_code);
    } else {
      sprintf(be_val, "dq %ld", be_num);
      char *be_const = genconst(prog, be_val);

      char *be_code = safe_alloc(1);
      be_code = safe_strcat(be_code, "mov rax, [rel ");
      be_code = safe_strcat(be_code, be_const);
      be_code = safe_strcat(be_code, "]\n");
      vector_append(fn->code, be_code);

      stack_push(fn, "rax");
      stack_rechar(fn, e->t_type, 1);
    }
    break;
  case UNOPEXPR:;
    unop_expr *ue = (unop_expr *)e->node;
    expr_asmgen(prog, fn, ue->rhs);

    switch (ue->rhs->t_type->type) {
    case FLOAT_T:
      stack_pop(fn, "xmm1");
      vector_append(fn->code, "pxor xmm0, xmm0\n");
      vector_append(fn->code, "subsd xmm0, xmm1\n");
      stack_push(fn, "xmm0");
      stack_rechar(fn, ue->rhs->t_type, 1);
      break;
    case INT_T:
      stack_pop(fn, "rax");
      vector_append(fn->code, "neg rax\n");
      stack_push(fn, "rax");
      stack_rechar(fn, ue->rhs->t_type, 1);
      break;
    case BOOL_T:
      stack_pop(fn, "rax");
      vector_append(fn->code, "xor rax, 1\n");
      stack_push(fn, "rax");
      stack_rechar(fn, ue->rhs->t_type, 1);
      break;
    default:
      ir_error("Invalid type in UNOP");
    }
    break;
  case BINOPEXPR:;
    binop_expr *boe = (binop_expr *)e->node;
    if (opt > 0 && is_opt_mult(boe)) {
      if (boe->lhs->type == INTEXPR &&
          is_pow_2(((int_expr *)boe->lhs->node)->val)) {
        expr_asmgen(prog, fn, boe->rhs);

        int_expr *lhs_ie = (int_expr *)boe->lhs->node;
        long shl_val = log2(lhs_ie->val);
        if (shl_val == 0)
          break;

        t *rhs_t = stack_pop(fn, "rax");
        if (!t_eq(rhs_t, boe->rhs->t_type))
          ir_error("Stack error in BINOPEXPR");

        char *code = safe_alloc(BUFSIZ);
        sprintf(code, "shl rax, %ld\n", shl_val);
        vector_append(fn->code, code);
        stack_push(fn, "rax");
      } else {
        expr_asmgen(prog, fn, boe->lhs);

        int_expr *rhs_ie = (int_expr *)boe->rhs->node;
        long shl_val = log2(rhs_ie->val);
        if (shl_val == 0)
          break;

        t *lhs_t = stack_pop(fn, "rax");
        if (!t_eq(lhs_t, boe->lhs->t_type))
          ir_error("Stack error in BINOPEXPR");

        char *code = safe_alloc(BUFSIZ);
        sprintf(code, "shl rax, %ld\n", shl_val);
        vector_append(fn->code, code);
        stack_push(fn, "rax");
      }
      break;
    }

    if (boe->op == MODOP && boe->lhs->t_type->type == FLOAT_T)
      stack_align(fn, 0);
    expr_asmgen(prog, fn, boe->rhs);
    expr_asmgen(prog, fn, boe->lhs);

    bool float_mode = boe->lhs->t_type->type == FLOAT_T;
    char *boe_lhs_reg = float_mode ? "xmm0" : "rax";
    char *boe_rhs_reg = float_mode ? "xmm1" : "r10";

    t *boe_lhs_t = stack_pop(fn, boe_lhs_reg);
    t *boe_rhs_t = stack_pop(fn, boe_rhs_reg);

    if (!t_eq(boe_lhs_t, boe->lhs->t_type) ||
        !t_eq(boe_rhs_t, boe->rhs->t_type))
      ir_error("Stack error in BINOPEXPR");

    char *boe_code = safe_alloc(1);
    switch (boe->op) {
    case ADDOP:
      if (float_mode)
        boe_code = safe_strcat(boe_code, "addsd xmm0, xmm1\n");
      else
        boe_code = safe_strcat(boe_code, "add rax, r10\n");
      break;
    case SUBOP:
      if (float_mode)
        boe_code = safe_strcat(boe_code, "subsd xmm0, xmm1\n");
      else
        boe_code = safe_strcat(boe_code, "sub rax, r10\n");
      break;
    case MULTOP:
      if (float_mode)
        boe_code = safe_strcat(boe_code, "mulsd xmm0, xmm1\n");
      else
        boe_code = safe_strcat(boe_code, "imul rax, r10\n");
      break;
    case DIVOP:
      if (float_mode)
        boe_code = safe_strcat(boe_code, "divsd xmm0, xmm1\n");
      else {
        boe_code = safe_strcat(boe_code, "cmp r10, 0\n");

        vector_append(fn->code, boe_code);
        boe_code = safe_alloc(1);
        assert_asmgen(prog, fn, "jne", "divide by zero");

        boe_code = safe_strcat(boe_code, "cqo\nidiv r10\n");
      }
      break;
    case MODOP:
      if (e->t_type->type == FLOAT_T) {
        boe_code = safe_strcat(boe_code, "call _fmod\n");
      } else {
        boe_code = safe_strcat(boe_code, "cmp r10, 0\n");

        vector_append(fn->code, boe_code);
        boe_code = safe_alloc(1);
        assert_asmgen(prog, fn, "jne", "mod by zero");

        boe_code = safe_strcat(boe_code, "cqo\nidiv r10\nmov rax, rdx\n");
      }
      break;
    case LTOP:
      if (float_mode)
        boe_code = safe_strcat(
            boe_code, "cmpltsd xmm0, xmm1\nmovq rax, xmm0\nand rax, 1\n");
      else
        boe_code = safe_strcat(boe_code, "cmp rax, r10\nsetl al\nand rax, 1\n");
      break;
    case GTOP:
      if (float_mode)
        boe_code = safe_strcat(
            boe_code, "cmpltsd xmm1, xmm0\nmovq rax, xmm1\nand rax, 1\n");
      else
        boe_code = safe_strcat(boe_code, "cmp rax, r10\nsetg al\nand rax, 1\n");
      break;
    case LEOP:
      if (float_mode)
        boe_code = safe_strcat(
            boe_code, "cmplesd xmm0, xmm1\nmovq rax, xmm0\nand rax, 1\n");
      else
        boe_code =
            safe_strcat(boe_code, "cmp rax, r10\nsetle al\nand rax, 1\n");
      break;
    case GEOP:
      if (float_mode)
        boe_code = safe_strcat(
            boe_code, "cmplesd xmm1, xmm0\nmovq rax, xmm1\nand rax, 1\n");
      else
        boe_code =
            safe_strcat(boe_code, "cmp rax, r10\nsetge al\nand rax, 1\n");
      break;
    case EQOP:
      if (float_mode)
        boe_code = safe_strcat(
            boe_code, "cmpeqsd xmm0, xmm1\nmovq rax, xmm0\nand rax, 1\n");
      else
        boe_code = safe_strcat(boe_code, "cmp rax, r10\nsete al\nand rax, 1\n");
      break;
    case NEOP:
      if (float_mode)
        boe_code = safe_strcat(
            boe_code, "cmpneqsd xmm0, xmm1\nmovq rax, xmm0\nand rax, 1\n");
      else
        boe_code =
            safe_strcat(boe_code, "cmp rax, r10\nsetne al\nand rax, 1\n");
      break;
    case ANDOP:
    case OROP:
      ir_error("BINOPEXPR not yet implemented");
    }
    vector_append(fn->code, boe_code);

    if (boe->op == MODOP && boe->lhs->t_type->type == FLOAT_T)
      stack_unalign(fn);

    if (e->t_type->type == FLOAT_T) {
      stack_push(fn, "xmm0");
      stack_rechar(fn, e->t_type, 1);
    } else {
      stack_push(fn, "rax");
      stack_rechar(fn, e->t_type, 1);
    }
    break;
  case ARRAYLITERALEXPR:;
    array_literal_expr *ale = (array_literal_expr *)e->node;
    for (long i = ale->exprs->size - 1; i >= 0; i--) {
      expr *cur_expr = vector_get_expr(ale->exprs, i);
      expr_asmgen(prog, fn, cur_expr);
    }

    array_info *ale_info = (array_info *)e->t_type->info;
    long ale_size = ale->exprs->size * sizeof_t(ale_info->type);
    char *ale_code = safe_alloc(BUFSIZ);
    sprintf(ale_code, "mov rdi, %ld\n", ale_size);
    vector_append(fn->code, ale_code);
    ale_code = safe_alloc(1);

    stack_align(fn, 0);
    vector_append(fn->code, "call _jpl_alloc\n");
    stack_unalign(fn);

    for (long i = ale_size - 8; i >= 0; i -= 8) {
      char *cur_offset = safe_alloc(BUFSIZ);
      sprintf(cur_offset, "%ld", i);

      ale_code = safe_strcat(ale_code, "mov r10, [rsp + ");
      ale_code = safe_strcat(ale_code, cur_offset);
      ale_code = safe_strcat(ale_code, "]\n");

      ale_code = safe_strcat(ale_code, "mov [rax + ");
      ale_code = safe_strcat(ale_code, cur_offset);
      ale_code = safe_strcat(ale_code, "], r10\n");
    }
    vector_append(fn->code, ale_code);
    fn->stk->shadow->size -= ale->exprs->size;
    fn->stk->size -= ale_size;
    ale_code = safe_alloc(BUFSIZ);
    sprintf(ale_code, "add rsp, %ld\n", ale_size);
    vector_append(fn->code, ale_code);
    stack_push(fn, "rax");

    ale_code = safe_alloc(BUFSIZ);
    sprintf(ale_code, "mov rax, %ld\n", ale->exprs->size);
    vector_append(fn->code, ale_code);
    stack_push(fn, "rax");

    stack_rechar(fn, e->t_type, 2);
    break;
  case VAREXPR:;
    var_expr *ve = (var_expr *)e->node;
    long ve_pos = stack_lookup(fn->stk, ve->var);

    if (ve_pos != (long)-1) {
      stack_alloc(fn, e->t_type);
      char *ve_start = safe_alloc(BUFSIZ);
      sprintf(ve_start, "rbp - %ld", ve_pos);
      stack_copy(fn, e->t_type, ve_start, "rsp");
      free(ve_start);
    } else if ((ve_pos = stack_lookup(prog->stk, ve->var)) != (long)-1) {
      stack_alloc(fn, e->t_type);
      char *ve_start = safe_alloc(BUFSIZ);
      sprintf(ve_start, "r12 - %ld", ve_pos);
      stack_copy(fn, e->t_type, ve_start, "rsp");
      free(ve_start);
    } else {
      char *msg = safe_alloc(BUFSIZ);
      sprintf(msg, "Could not find var '%s' on stack", ve->var);
      ir_error(msg);
    }
    break;
  case CALLEXPR:;
    call_expr *ce = (call_expr *)e->node;

    // Find asm_fn
    asm_fn *ce_fn = NULL;
    if (!strcmp(fn->name, ce->var)) {
      ce_fn = fn;
    } else {
      for (long i = 0; i < prog->fns->size; i++) {
        asm_fn *cur = vector_get_asm_fn(prog->fns, i);
        if (!strcmp(ce->var, cur->name)) {
          ce_fn = cur;
          break;
        }
      }

      if (ce_fn == NULL) {
        char *msg = safe_alloc(BUFSIZ);
        sprintf(msg, "Could not find function '%s'", ce->var);
        ir_error(msg);
      }
    }
    call_conv *call = ce_fn->call;

    // Find out if ret on stack
    bool stack_ret = strcmp(call->ret, "rax") && strcmp(call->ret, "xmm0");

    // Prepare stack
    long ret_pos = 0;
    if (stack_ret) {
      stack_alloc(fn, call->ret_t);
      ret_pos = fn->stk->size;
    }
    stack_align(fn, 0);

    // generate code for args
    // stack args
    for (long i = ce->exprs->size - 1; i >= 0; i--) {
      expr *cur_expr = vector_get_expr(ce->exprs, i);
      char *cur = vector_get_str(call->args, i);
      if (!is_int_reg(cur) && !is_float_reg(cur))
        expr_asmgen(prog, fn, cur_expr);
    }
    // other args
    for (long i = ce->exprs->size - 1; i >= 0; i--) {
      expr *cur_expr = vector_get_expr(ce->exprs, i);
      char *cur = vector_get_str(call->args, i);
      if (is_int_reg(cur) || is_float_reg(cur))
        expr_asmgen(prog, fn, cur_expr);
    }

    // pop args
    for (long i = 0; i < ce->exprs->size; i++) {
      char *cur = vector_get_str(call->args, i);
      if (is_int_reg(cur) || is_float_reg(cur))
        stack_pop(fn, cur);
    }

    // do call
    char *ce_code = safe_alloc(BUFSIZ);
    if (stack_ret) {
      long offset = fn->stk->size - ret_pos;
      sprintf(ce_code, "lea rdi, [rsp + %ld]\n", offset);
    }

    ce_code = safe_strcat(ce_code, "call _");
    ce_code = safe_strcat(ce_code, ce->var);
    ce_code = safe_strcat(ce_code, "\n");
    vector_append(fn->code, ce_code);

    // free stack
    for (long i = ce->exprs->size - 1; i >= 0; i--) {
      char *cur = vector_get_str(call->args, i);
      expr *cur_expr = vector_get_expr(ce->exprs, i);
      if (!is_int_reg(cur) && !is_float_reg(cur)) {
        stack_free(fn, sizeof_t(cur_expr->t_type));
      }
    }
    stack_unalign(fn);

    // push ret
    if (!stack_ret) {
      switch (call->ret_t->type) {
      case FLOAT_T:
        stack_push(fn, "xmm0");
        break;
      case INT_T:
      case BOOL_T:
      case VOID_T:
        stack_push(fn, "rax");
        break;
      default:
        ir_error("Invalid register return type in CALLEXPR");
      }
      stack_rechar(fn, call->ret_t, 1);
    }
    break;
  case IFEXPR:;
    if_expr *ife = (if_expr *)e->node;

    expr_asmgen(prog, fn, ife->if_expr);
    if (opt > 0 && is_bool_cast(ife)) {
      stack_rechar(fn, e->t_type, 1);
      break;
    }

    stack_pop(fn, "rax");
    vector_append(fn->code, "cmp rax, 0\n");

    char *else_jmp = jmp_asmgen(prog);
    char *end_jmp = jmp_asmgen(prog);
    char *ife_code = safe_alloc(1);

    ife_code = safe_strcat(ife_code, "je ");
    ife_code = safe_strcat(ife_code, else_jmp);
    ife_code = safe_strcat(ife_code, "\n");
    vector_append(fn->code, ife_code);
    ife_code = safe_alloc(1);

    expr_asmgen(prog, fn, ife->then_expr);
    fn->stk->size -= sizeof_t(e->t_type);
    fn->stk->shadow->size -= 1;

    ife_code = safe_strcat(ife_code, "jmp ");
    ife_code = safe_strcat(ife_code, end_jmp);
    ife_code = safe_strcat(ife_code, "\n");

    ife_code = safe_strcat(ife_code, else_jmp);
    ife_code = safe_strcat(ife_code, ":\n");
    vector_append(fn->code, ife_code);
    ife_code = safe_alloc(1);

    expr_asmgen(prog, fn, ife->else_expr);
    ife_code = safe_strcat(ife_code, end_jmp);
    ife_code = safe_strcat(ife_code, ":\n");
    vector_append(fn->code, ife_code);
    break;
  case ARRAYINDEXEXPR:;
    array_index_expr *aie = (array_index_expr *)e->node;
    array_info *aie_info = (array_info *)aie->expr->t_type->info;
    long gap_aie = 8 * aie_info->rank;

    bool local_arr = false;
    if (opt > 0 && aie->expr->type == VAREXPR) {
      var_expr *ve = (var_expr *)aie->expr->node;
      long offset = stack_lookup(fn->stk, ve->var);
      gap_aie = (fn->stk->size - offset + gap_aie);
      local_arr = true;
    } else {
      expr_asmgen(prog, fn, aie->expr);
    }

    for (long i = aie->exprs->size - 1; i >= 0; i--) {
      expr *cur_expr = vector_get_expr(aie->exprs, i);
      expr_asmgen(prog, fn, cur_expr);
    }

    for (long i = 0; i < aie->exprs->size; i++) {
      char *code = safe_alloc(BUFSIZ);
      sprintf(code, "mov rax, [rsp + %ld]\n", i * 8);
      code = safe_strcat(code, "cmp rax, 0\n");
      vector_append(fn->code, code);
      code = safe_alloc(BUFSIZ);
      assert_asmgen(prog, fn, "jge", "negative array index");

      sprintf(code, "cmp rax, [rsp + %ld]\n", (gap_aie + i * 8));
      vector_append(fn->code, code);
      assert_asmgen(prog, fn, "jl", "index too large");
    }

    long i_aie = 0;

    if (opt > 0) {
      char *code = safe_alloc(BUFSIZ);
      sprintf(code, "mov rax, [rsp + 0]\n");
      vector_append(fn->code, code);
      i_aie = 1;
    } else {
      vector_append(fn->code, "mov rax, 0\n");
    }

    for (; i_aie < aie->exprs->size; i_aie++) {
      char *code = safe_alloc(BUFSIZ);
      sprintf(code, "imul rax, [rsp + %ld]\n", i_aie * 8 + gap_aie);
      vector_append(fn->code, code);

      code = safe_alloc(BUFSIZ);
      sprintf(code, "add rax, [rsp + %ld]\n", i_aie * 8);
      vector_append(fn->code, code);
    }

    char *code = safe_alloc(BUFSIZ);
    if (opt > 0 && is_pow_2(sizeof_t(aie_info->type)))
      sprintf(code, "shl rax, %ld\n", (long)log2(sizeof_t(aie_info->type)));
    else
      sprintf(code, "imul rax, %ld\n", sizeof_t(aie_info->type));
    vector_append(fn->code, code);

    code = safe_alloc(BUFSIZ);
    sprintf(code, "add rax, [rsp + %ld]\n", aie->exprs->size * 8 + gap_aie);
    vector_append(fn->code, code);
    if (opt > 0) {
      long space = 0;
      for (long i = 0; i < aie->exprs->size; i++) {
        expr *cur = vector_get_expr(aie->exprs, i);
        fn->stk->shadow->size -= 1;
        fn->stk->size -= sizeof_t(cur->t_type);
        space += sizeof_t(cur->t_type);
      }

      char *code = safe_alloc(BUFSIZ);
      sprintf(code, "add rsp, %ld\n", space);
      vector_append(fn->code, code);
    } else {
      for (long i = 0; i < aie->exprs->size; i++) {
        expr *cur = vector_get_expr(aie->exprs, i);
        t *type = stack_pop(fn, NULL);
        if (!t_eq(cur->t_type, type))
          ir_error("Stack error in INDEXEXPR");
      }
    }
    if (!local_arr) {
      t *aie_e_t = stack_pop(fn, NULL);
      if (!t_eq(aie_e_t, aie->expr->t_type))
        ir_error("Stack error in INDEXEXPR");
    }

    stack_alloc(fn, aie_info->type);
    stack_copy(fn, aie_info->type, "rax", "rsp");
    break;
  case SUMLOOPEXPR:;
    sum_loop_expr *sloop = (sum_loop_expr *)e->node;
    stack_alloc(fn, e->t_type);

    // Bounds
    for (long i = sloop->exprs->size - 1; i >= 0; i--) {
      expr *cur = vector_get_expr(sloop->exprs, i);
      expr_asmgen(prog, fn, cur);

      vector_append(fn->code, "mov rax, [rsp]\ncmp rax, 0\n");
      assert_asmgen(prog, fn, "jg", "non-positive loop bound");
    }

    char *sloop_code = safe_alloc(BUFSIZ);
    sprintf(sloop_code, "mov rax, 0\nmov [rsp + %ld], rax\n",
            sloop->exprs->size * 8);
    vector_append(fn->code, sloop_code);

    for (long i = sloop->exprs->size - 1; i >= 0; i--) {
      vector_append(fn->code, "mov rax, 0\n");
      stack_push(fn, "rax");

      lval *lv = safe_alloc(sizeof(lval));
      lv->type = VARLVALUE;
      lv->node = safe_alloc(sizeof(var_lval));

      var_lval *vlv = (var_lval *)lv->node;
      vlv->var = vector_get_str(sloop->vars, i);
      push_lval(fn, lv, fn->stk->size);
    }

    // Body
    char *sloop_body = jmp_asmgen(prog);
    sloop_code = safe_alloc(strlen(sloop_body) + 3);
    sprintf(sloop_code, "%s:\n", sloop_body);
    vector_append(fn->code, sloop_code);
    expr_asmgen(prog, fn, sloop->expr);

    sloop_code = safe_alloc(BUFSIZ);
    long target = 2 * sloop->exprs->size * 8;
    switch (sloop->expr->t_type->type) {
    case INT_T:
      stack_pop(fn, "rax");
      sprintf(sloop_code, "add [rsp + %ld], rax\n", target);
      break;
    case FLOAT_T:
      stack_pop(fn, "xmm0");
      sprintf(sloop_code, "addsd xmm0, [rsp + %ld]\nmovsd [rsp + %ld], xmm0\n",
              target, target);
      break;
    default:
      ir_error("Unexpected EXPR type in SUMLOOPEXPR");
    }
    vector_append(fn->code, sloop_code);

    sloop_code = safe_alloc(BUFSIZ);
    sprintf(sloop_code, "add qword [rsp + %ld], 1\n",
            (sloop->exprs->size - 1) * 8);
    vector_append(fn->code, sloop_code);

    for (long i = sloop->vars->size - 1; i >= 0; i--) {
      sloop_code = safe_alloc(BUFSIZ);
      sprintf(sloop_code, "mov rax, [rsp + %ld]\ncmp rax, [rsp + %ld]\njl %s\n",
              i * 8, (i + sloop->exprs->size) * 8, sloop_body);
      vector_append(fn->code, sloop_code);

      if (i != 0) {
        sloop_code = safe_alloc(BUFSIZ);
        sprintf(sloop_code,
                "mov qword [rsp + %ld], 0\nadd qword [rsp + %ld], 1\n", i * 8,
                (i - 1) * 8);
        vector_append(fn->code, sloop_code);
      }
    }

    long free = sloop->exprs->size * 8;
    stack_free(fn, free);
    stack_free(fn, free);
    break;
  case ARRAYLOOPEXPR:;
    array_loop_expr *aloop = (array_loop_expr *)e->node;

    // Alloc pointer
    t *int_t = safe_alloc(sizeof(t));
    int_t->type = INT_T;
    int_t->info = NULL;
    stack_alloc(fn, int_t);

    // Handle tensor contractions
    if (opt >= 3 && is_tc(aloop)) {
      graph *g = build_tc_graph(aloop);
      vector *topo = build_topo_order(g);
      sum_loop_expr *sloop = (sum_loop_expr *)aloop->expr;

      // Push bounds
      for (int i = 0; i < aloop->exprs->size; i++) {
        expr *cur = vector_get_expr(aloop->exprs, i);
        expr_asmgen(prog, fn, cur);

        vector_append(fn->code, "mov rax, [rsp]\ncmp rax, 0\n");
        assert_asmgen(prog, fn, "jg", "non-positive loop bound");
      }
      for (long i = sloop->exprs->size - 1; i >= 0; i--) {
        expr *cur = vector_get_expr(sloop->exprs, i);
        expr_asmgen(prog, fn, cur);

        vector_append(fn->code, "mov rax, [rsp]\ncmp rax, 0\n");
        assert_asmgen(prog, fn, "jg", "non-positive loop bound");
      }

      // Alloc space
      array_info *aloop_info = (array_info *)e->t_type->info;
      char *aloop_code = safe_alloc(BUFSIZ);
      sprintf(aloop_code, "mov rdi, %lu\n", sizeof_t(aloop_info->type));
      vector_append(fn->code, aloop_code);

      for (long i = 0; i < aloop->exprs->size; i++) {
        aloop_code = safe_alloc(BUFSIZ);
        sprintf(aloop_code, "imul rdi, [rsp + %ld]\n",
                i * 8 + (sloop->exprs->size * 8));
        vector_append(fn->code, aloop_code);
        assert_asmgen(prog, fn, "jno", "overflow computing array size");
      }

      stack_align(fn, 0);
      vector_append(fn->code, "call _jpl_alloc\n");
      stack_unalign(fn);

      // Move pointer to alloced space
      aloop_code = safe_alloc(BUFSIZ);
      sprintf(aloop_code, "mov [rsp + %ld], rax\n", aloop->exprs->size * 8);
      vector_append(fn->code, aloop_code);

      // Init bounds

      // Begin loop

      // Compute sum body

      // Compute array index

      // Add body to computed index

      // Increment (topo ordering)

      // free (not array bounds)

    }

    else {

      // Bounds
      for (long i = aloop->exprs->size - 1; i >= 0; i--) {
        expr *cur = vector_get_expr(aloop->exprs, i);
        expr_asmgen(prog, fn, cur);

        vector_append(fn->code, "mov rax, [rsp]\ncmp rax, 0\n");
        assert_asmgen(prog, fn, "jg", "non-positive loop bound");
      }

      // Compute array size
      array_info *aloop_info = (array_info *)e->t_type->info;
      char *aloop_code = safe_alloc(BUFSIZ);
      sprintf(aloop_code, "mov rdi, %lu\n", sizeof_t(aloop_info->type));
      vector_append(fn->code, aloop_code);

      for (long i = 0; i < aloop->exprs->size; i++) {
        aloop_code = safe_alloc(BUFSIZ);
        sprintf(aloop_code, "imul rdi, [rsp + %ld]\n", i * 8);
        vector_append(fn->code, aloop_code);
        assert_asmgen(prog, fn, "jno", "overflow computing array size");
      }

      stack_align(fn, 0);
      vector_append(fn->code, "call _jpl_alloc\n");
      stack_unalign(fn);

      // Initialize array
      aloop_code = safe_alloc(BUFSIZ);
      sprintf(aloop_code, "mov [rsp + %ld], rax\n", aloop->exprs->size * 8);
      vector_append(fn->code, aloop_code);
      for (long i = aloop->exprs->size - 1; i >= 0; i--) {
        vector_append(fn->code, "mov rax, 0\n");
        stack_push(fn, "rax");

        lval *lv = safe_alloc(sizeof(lval));
        lv->type = VARLVALUE;
        lv->node = safe_alloc(sizeof(var_lval));

        var_lval *vlv = (var_lval *)lv->node;
        vlv->var = vector_get_str(aloop->vars, i);
        push_lval(fn, lv, fn->stk->size);
      }

      // Body
      char *aloop_body = jmp_asmgen(prog);
      aloop_code = safe_alloc(strlen(aloop_body) + 3);
      sprintf(aloop_code, "%s:\n", aloop_body);
      vector_append(fn->code, aloop_code);
      expr_asmgen(prog, fn, aloop->expr);

      // Result index
      long offset = sizeof_t(aloop->expr->t_type);
      long i_aloop = 0;

      if (opt > 0) {
        char *code = safe_alloc(BUFSIZ);
        sprintf(code, "mov rax, [rsp + %ld]\n", offset);
        vector_append(fn->code, code);
        i_aloop = 1;
      } else {
        vector_append(fn->code, "mov rax, 0\n");
      }

      for (; i_aloop < aloop->exprs->size; i_aloop++) {
        aloop_code = safe_alloc(BUFSIZ);

        expr *cur_expr = vector_get_expr(aloop->exprs, i_aloop);
        int_expr *cur_ie = NULL;
        if (cur_expr->type == INTEXPR)
          cur_ie = (int_expr *)cur_expr->node;

        if (opt > 0 && cur_ie != NULL &&
            ((cur_ie->val <= INT_MAX && cur_ie->val >= INT_MIN) ||
             is_pow_2(cur_ie->val))) {
          long mul_val = cur_ie->val;
          long add_val = offset + i_aloop * 8;
          if (is_pow_2(mul_val)) {
            sprintf(aloop_code, "shl rax, %d\nadd rax, [rsp + %ld]\n",
                    (int)log2(mul_val), add_val);
          } else {
            sprintf(aloop_code, "imul rax, %d\nadd rax, [rsp + %ld]\n",
                    (int)mul_val, add_val);
          }
        } else {
          long mul_val = offset + i_aloop * 8 + aloop_info->rank * 8;
          long add_val = offset + i_aloop * 8;
          sprintf(aloop_code, "imul rax, [rsp + %ld]\nadd rax, [rsp + %ld]\n",
                  mul_val, add_val);
        }
        vector_append(fn->code, aloop_code);
      }

      aloop_code = safe_alloc(BUFSIZ);
      if (opt > 0 && is_pow_2(sizeof_t(aloop_info->type)))
        sprintf(aloop_code, "shl rax, %ld\nadd rax, [rsp + %ld]\n",
                (long)log2(sizeof_t(aloop_info->type)),
                offset + aloop->exprs->size * 8 + aloop_info->rank * 8);
      else
        sprintf(aloop_code, "imul rax, %ld\nadd rax, [rsp + %ld]\n",
                sizeof_t(aloop_info->type),
                offset + aloop->exprs->size * 8 + aloop_info->rank * 8);
      vector_append(fn->code, aloop_code);

      // Copy data
      stack_copy(fn, aloop_info->type, "rsp", "rax");
      stack_pop(fn, NULL);

      // Incr bounds
      aloop_code = safe_alloc(BUFSIZ);
      sprintf(aloop_code, "add qword [rsp + %ld], 1\n",
              (aloop->exprs->size - 1) * 8);
      vector_append(fn->code, aloop_code);

      for (long i = aloop->vars->size - 1; i >= 0; i--) {
        aloop_code = safe_alloc(BUFSIZ);
        sprintf(aloop_code,
                "mov rax, [rsp + %ld]\ncmp rax, [rsp + %ld]\njl %s\n", i * 8,
                (i + aloop->exprs->size) * 8, aloop_body);
        vector_append(fn->code, aloop_code);

        if (i != 0) {
          aloop_code = safe_alloc(BUFSIZ);
          sprintf(aloop_code,
                  "mov qword [rsp + %ld], 0\nadd qword [rsp + %ld], 1\n", i * 8,
                  (i - 1) * 8);
          vector_append(fn->code, aloop_code);
        }
      }

      // Free
      stack_free(fn, aloop->exprs->size * 8);

      t *array_t = safe_alloc(sizeof(t));
      array_t->type = ARRAY_T;
      array_t->info = aloop_info;
      stack_rechar(fn, array_t, aloop_info->rank + 1);
    }

    break;
  default:
    ir_error("EXPR is not implemented yet");
  }
}

bool is_bool_cast(if_expr *ife) {
  if (ife->then_expr->type != INTEXPR)
    return false;
  int_expr *then_expr = (int_expr *)ife->then_expr->node;
  if (then_expr->val != 1)
    return false;

  if (ife->else_expr->type != INTEXPR)
    return false;
  int_expr *else_expr = (int_expr *)ife->else_expr->node;
  if (else_expr->val != 0)
    return false;

  return true;
}

bool is_pow_2(long n) { return n > 0 && (n & (n - 1)) == 0; }

bool is_opt_mult(binop_expr *boe) {
  if (boe->op != MULTOP)
    return false;

  if (boe->lhs->type != INTEXPR && boe->rhs->type != INTEXPR)
    return false;

  if (boe->lhs->type == INTEXPR && boe->rhs->type == INTEXPR) {
    int_expr *lhs_ie = (int_expr *)boe->lhs->node;
    int_expr *rhs_ie = (int_expr *)boe->rhs->node;

    if (!is_pow_2(lhs_ie->val) && !is_pow_2(rhs_ie->val))
      return false;
  } else if (boe->lhs->type == INTEXPR) {
    int_expr *lhs_ie = (int_expr *)boe->lhs->node;
    if (!is_pow_2(lhs_ie->val))
      return false;
  } else {
    int_expr *rhs_ie = (int_expr *)boe->rhs->node;
    if (!is_pow_2(rhs_ie->val))
      return false;
  }

  return true;
}

long log2(long n) {
  int log = 0;
  while ((n /= 2) > 0) {
    log++;
  }
  return log;
}

bool is_tc(array_loop_expr *aloop) {
  if (aloop->expr->type != SUMLOOPEXPR)
    return false;

  if (aloop->expr->t_type->type != FLOAT_T)
    return false;

  sum_loop_expr *sloop = (sum_loop_expr *)aloop->expr->node;
  if (!is_tc_body(sloop->expr))
    return false;

  return true;
}

bool is_tc_body(expr *e) {
  if (e->type == BINOPEXPR) {
    binop_expr *bop = (binop_expr *)e->node;
    if (is_tc_body(bop->lhs) && is_tc_body(bop->rhs))
      return true;
  }

  if (e->type == ARRAYINDEXEXPR) {
    array_index_expr *aie = (array_index_expr *)e->node;
    if (is_tc_primitive(aie->expr)) {
      for (int i = 0; i < aie->exprs->size; i++) {
        expr *cur = vector_get_expr(aie->exprs, i);
        if (!is_tc_primitive(cur))
          return false;
      }
      return true;
    }
  }

  if (is_tc_primitive(e))
    return true;

  return false;
}

bool is_tc_primitive(expr *e) {
  if (e->type == INTEXPR || e->type == FLOATEXPR || e->type == VAREXPR)
    return true;

  return false;
}

graph *build_tc_graph(array_loop_expr *aloop) {
  graph *g = safe_alloc(sizeof(graph));
  g->nodes = safe_alloc(sizeof(vector));
  g->edges_from = safe_alloc(sizeof(vector));
  g->edges_to = safe_alloc(sizeof(vector));
  vector_init(g->nodes, 8, STRVECTOR);
  vector_init(g->edges_from, 8, STRVECTOR);
  vector_init(g->edges_to, 8, STRVECTOR);

  for (int i = 0; i < aloop->vars->size; i++) {
    char *cur = vector_get_str(aloop->vars, i);
    vector_append(g->nodes, cur);

    if (i != aloop->vars->size - 1) {
      for (int j = i + 1; j < aloop->vars->size; j++) {
        char *cur_j = vector_get_str(aloop->vars, j);
        vector_append(g->edges_from, cur);
        vector_append(g->edges_to, cur_j);
      }
    }
  }

  sum_loop_expr *sloop = (sum_loop_expr *)aloop->expr->node;
  for (int i = 0; i < sloop->vars->size; i++) {
    char *cur = vector_get_str(sloop->vars, i);
    vector_append(g->nodes, cur);

    if (i != sloop->vars->size - 1) {
      for (int j = i + 1; j < sloop->vars->size; j++) {
        char *cur_j = vector_get_str(sloop->vars, j);
        vector_append(g->edges_from, cur);
        vector_append(g->edges_to, cur_j);
      }
    }
  }

  build_tc_edges(g, sloop->expr);

  // Delete duplicate edges
  vector *found_from = safe_alloc(sizeof(vector));
  vector *found_to = safe_alloc(sizeof(vector));
  vector_init(found_from, g->edges_from->size, STRVECTOR);
  vector_init(found_to, g->edges_to->size, STRVECTOR);

  for (int i = 0; i < g->edges_from->size; i++) {
    char *from = vector_get_str(g->edges_from, i);
    char *to = vector_get_str(g->edges_to, i);

    bool found = false;
    for (int j = 0; j < found_from->size; j++) {
      char *cur_from = vector_get_str(found_from, j);
      if (!strcmp(cur_from, from)) {
        char *cur_to = vector_get_str(found_to, j);
        if (!strcmp(cur_to, to)) {
          found = true;
          break;
        }
      }
    }

    if (!found) {
      vector_append(found_from, from);
      vector_append(found_to, to);
    }
  }

  free(g->edges_from);
  free(g->edges_to);
  g->edges_from = found_from;
  g->edges_to = found_to;

  return g;
}

void build_tc_edges(graph *g, expr *body) {
  switch (body->type) {
  case BINOPEXPR:;
    binop_expr *bop = (binop_expr *)body->node;
    build_tc_edges(g, bop->lhs);
    build_tc_edges(g, bop->rhs);
    break;
  case ARRAYINDEXEXPR:;
    array_index_expr *aie = (array_index_expr *)body->node;
    for (int i = 0; i < aie->exprs->size; i++) {
      expr *cur = vector_get_expr(aie->exprs, i);
      if (cur->type != VAREXPR)
        continue;

      var_expr *ve = (var_expr *)cur->node;
      if (!vector_contains_str(g->nodes, ve->var))
        continue;

      if (i != aie->exprs->size - 1) {
        for (int j = i + 1; j < aie->exprs->size; j++) {
          expr *cur_j = vector_get_expr(aie->exprs, j);
          if (cur_j->type != VAREXPR)
            continue;

          var_expr *ve_j = (var_expr *)cur_j->node;
          if (!vector_contains_str(g->nodes, ve_j->var))
            continue;

          vector_append(g->edges_from, ve->var);
          vector_append(g->edges_to, ve_j->var);
        }
      }
    }
    break;
  default:
    break;
  }
}

vector *build_topo_order(graph *g) {
  vector *v = safe_alloc(sizeof(vector));
  vector_init(v, g->nodes->size, STRVECTOR);

  int loops = 0;
  while (v->size != g->nodes->size) {
    loops += 1;
    // Should only happen if not DAG
    if (loops > g->nodes->size)
      return NULL;

    for (int i = 0; i < g->nodes->size; i++) {
      char *cur = vector_get_str(g->nodes, i);
      if (cur == NULL)
        continue;

      bool dst = false;
      for (int j = 0; j < g->edges_to->size; j++) {
        char *cur_to = vector_get_str(g->edges_to, j);
        if (cur_to != NULL && !strcmp(cur_to, cur)) {
          dst = true;
          break;
        }
      }

      if (!dst) {
        vector_append(v, cur);

        for (int j = 0; j < g->edges_from->size; j++) {
          char *cur_from = vector_get_str(g->edges_from, j);
          if (cur_from != NULL && !strcmp(cur_from, cur)) {
            g->edges_from->data[j] = NULL;
            g->edges_to->data[j] = NULL;
          }
          g->nodes->data[i] = NULL;
        }
        break;
      }
    }
  }

  free(g->nodes);
  free(g->edges_to);
  free(g->edges_from);
  free(g);

  return v;
}
