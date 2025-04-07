#include "asm_ir.h"
#include "compiler_error.h"
#include "safe.h"
#include "vector_get.h"
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
    char *ie_const = genconst(prog, ie_val);

    char *ie_code = safe_alloc(1);
    ie_code = safe_strcat(ie_code, "mov rax, [rel ");
    ie_code = safe_strcat(ie_code, ie_const);
    ie_code = safe_strcat(ie_code, "]\n");
    vector_append(fn->code, ie_code);

    stack_push(fn, "rax");
    stack_rechar(fn, e->t_type, 1);
    break;
  case TRUEEXPR:
  case FALSEEXPR:;
    char *be_val = safe_alloc(BUFSIZ);
    int be_num = e->type == TRUEEXPR ? 1 : 0;
    sprintf(be_val, "dq %d", be_num);
    char *be_const = genconst(prog, be_val);

    char *be_code = safe_alloc(1);
    be_code = safe_strcat(be_code, "mov rax, [rel ");
    be_code = safe_strcat(be_code, be_const);
    be_code = safe_strcat(be_code, "]\n");
    vector_append(fn->code, be_code);

    stack_push(fn, "rax");
    stack_rechar(fn, e->t_type, 1);
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
    for (int i = ale->exprs->size - 1; i >= 0; i--) {
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
    sprintf(ale_code, "mov rax, %d\n", ale->exprs->size);
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
      for (int i = 0; i < prog->fns->size; i++) {
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
    for (int i = ce->exprs->size - 1; i >= 0; i--) {
      expr *cur_expr = vector_get_expr(ce->exprs, i);
      char *cur = vector_get_str(call->args, i);
      if (!is_int_reg(cur) && !is_float_reg(cur))
        expr_asmgen(prog, fn, cur_expr);
    }
    // other args
    for (int i = ce->exprs->size - 1; i >= 0; i--) {
      expr *cur_expr = vector_get_expr(ce->exprs, i);
      char *cur = vector_get_str(call->args, i);
      if (is_int_reg(cur) || is_float_reg(cur))
        expr_asmgen(prog, fn, cur_expr);
    }

    // pop args
    for (int i = 0; i < ce->exprs->size; i++) {
      char *cur = vector_get_str(call->args, i);
      if (is_int_reg(cur) || is_float_reg(cur))
        stack_pop(fn, cur);
    }

    // do call
    char *ce_code = safe_alloc(BUFSIZ);
    if (stack_ret) {
      long offset = fn->stk->size - ret_pos;

      for (int i = fn->stk->shadow->size - 1; i >= 0; i--) {
        t *cur = vector_get_t(fn->stk->shadow, i);
        if (cur->type == PAD_T) {
          padding *p = (padding *)cur->info;
          offset += p->size;
          break;
        }
      }

      sprintf(ce_code, "lea rdi, [rsp + %ld]\n", offset);
    }

    ce_code = safe_strcat(ce_code, "call _");
    ce_code = safe_strcat(ce_code, ce->var);
    ce_code = safe_strcat(ce_code, "\n");
    vector_append(fn->code, ce_code);

    // free stack
    for (int i = ce->exprs->size - 1; i >= 0; i--) {
      char *cur = vector_get_str(call->args, i);
      if (!is_int_reg(cur) && !is_float_reg(cur)) {
        stack_pop(fn, NULL);
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
    expr_asmgen(prog, fn, aie->expr);

    array_info *aie_info = (array_info *)aie->expr->t_type->info;
    for (int i = aie->exprs->size - 1; i >= 0; i--) {
      expr *cur_expr = vector_get_expr(aie->exprs, i);
      expr_asmgen(prog, fn, cur_expr);
    }

    for (int i = 0; i < aie->exprs->size; i++) {
      char *code = safe_alloc(BUFSIZ);
      sprintf(code, "mov rax, [rsp + %d]\n", i * 8);
      code = safe_strcat(code, "cmp rax, 0\n");
      vector_append(fn->code, code);
      code = safe_alloc(BUFSIZ);
      assert_asmgen(prog, fn, "jge", "negative array index");

      sprintf(code, "cmp rax, [rsp + %d]\n", (i + aie_info->rank) * 8);
      vector_append(fn->code, code);
      assert_asmgen(prog, fn, "jl", "index too large");
    }

    vector_append(fn->code, "mov rax, 0\n");
    for (int i = 0; i < aie->exprs->size; i++) {
      char *code = safe_alloc(BUFSIZ);
      sprintf(code, "imul rax, [rsp + %d]\n", i * 8 + aie_info->rank * 8);
      vector_append(fn->code, code);

      code = safe_alloc(BUFSIZ);
      sprintf(code, "add rax, [rsp + %d]\n", i * 8);
      vector_append(fn->code, code);
    }

    char *code = safe_alloc(BUFSIZ);
    sprintf(code, "imul rax, %ld\n", sizeof_t(aie_info->type));
    vector_append(fn->code, code);

    code = safe_alloc(BUFSIZ);
    sprintf(code, "add rax, [rsp + %d]\n",
            aie->exprs->size * 8 + aie_info->rank * 8);
    vector_append(fn->code, code);

    for (int i = 0; i < aie->exprs->size; i++) {
      expr *cur = vector_get_expr(aie->exprs, i);
      t *type = stack_pop(fn, NULL);
      if (!t_eq(cur->t_type, type))
        ir_error("Stack error in INDEXEXPR");
    }
    t *aie_e_t = stack_pop(fn, NULL);
    if (!t_eq(aie_e_t, aie->expr->t_type))
      ir_error("Stack error in INDEXEXPR");

    stack_alloc(fn, aie_info->type);
    stack_copy(fn, aie_info->type, "rax", "rsp");
    break;
  case SUMLOOPEXPR:;
    sum_loop_expr *sloop = (sum_loop_expr *)e->node;
    stack_alloc(fn, e->t_type);

    // Bounds
    for (int i = sloop->exprs->size - 1; i >= 0; i--) {
      expr *cur = vector_get_expr(sloop->exprs, i);
      expr_asmgen(prog, fn, cur);

      vector_append(fn->code, "mov rax, [rsp]\ncmp rax, 0\n");
      assert_asmgen(prog, fn, "jg", "non-positive loop bound");
    }

    char *sloop_code = safe_alloc(BUFSIZ);
    sprintf(sloop_code, "mov rax, 0\nmov [rsp + %d], rax\n",
            sloop->exprs->size * 8);
    vector_append(fn->code, sloop_code);

    for (int i = sloop->exprs->size - 1; i >= 0; i--) {
      vector_append(fn->code, "mov rax, 0\n");
      stack_push(fn, "rax");
      t *int_t = safe_alloc(sizeof(t));
      int_t->type = INT_T;
      int_t->info = NULL;
      stack_rechar(fn, int_t, 1);

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
    int target = 2 * sloop->exprs->size * 8;
    switch (sloop->expr->t_type->type) {
    case INT_T:
      stack_pop(fn, "rax");
      sprintf(sloop_code, "add [rsp + %d], rax\n", target);
      break;
    case FLOAT_T:
      stack_pop(fn, "xmm0");
      sprintf(sloop_code, "addsd xmm0, [rsp + %d]\nmovsd [rsp + %d], xmm0\n",
              target, target);
      break;
    default:
      ir_error("Unexpected EXPR type in SUMLOOPEXPR");
    }
    vector_append(fn->code, sloop_code);

    sloop_code = safe_alloc(BUFSIZ);
    sprintf(sloop_code, "add qword [rsp + %d], 1\n",
            (sloop->exprs->size - 1) * 8);
    vector_append(fn->code, sloop_code);

    for (int i = sloop->vars->size - 1; i >= 0; i--) {
      sloop_code = safe_alloc(BUFSIZ);
      sprintf(sloop_code, "mov rax, [rsp + %d]\ncmp rax, [rsp + %d]\njl %s\n",
              i * 8, (i + sloop->exprs->size) * 8, sloop_body);
      vector_append(fn->code, sloop_code);

      if (i != 0) {
        sloop_code = safe_alloc(BUFSIZ);
        sprintf(sloop_code,
                "mov qword [rsp + %d], 0\nadd qword [rsp + %d], 1\n", i * 8,
                (i - 1) * 8);
        vector_append(fn->code, sloop_code);
      }
    }

    int free = sloop->exprs->size * 8;
    stack_free(fn, free);
    stack_free(fn, free);
    break;
  case ARRAYLOOPEXPR:;
    array_loop_expr *aloop = (array_loop_expr *)e->node;
    t *int_t = safe_alloc(sizeof(t));
    int_t->type = INT_T;
    int_t->info = NULL;
    stack_alloc(fn, int_t);

    // Bounds
    for (int i = aloop->exprs->size - 1; i >= 0; i--) {
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

    for (int i = 0; i < aloop->exprs->size; i++) {
      aloop_code = safe_alloc(BUFSIZ);
      sprintf(aloop_code, "imul rdi, [rsp + %d]\n", i * 8);
      vector_append(fn->code, aloop_code);
      assert_asmgen(prog, fn, "jno", "overflow computing array size");
    }

    stack_align(fn, 0);
    vector_append(fn->code, "call _jpl_alloc\n");
    stack_unalign(fn);

    // Initialize array
    aloop_code = safe_alloc(BUFSIZ);
    sprintf(aloop_code, "mov [rsp + %d], rax\n", aloop->exprs->size * 8);
    vector_append(fn->code, aloop_code);
    for (int i = aloop->exprs->size - 1; i >= 0; i--) {
      vector_append(fn->code, "mov rax, 0\n");
      stack_push(fn, "rax");
      stack_rechar(fn, int_t, 1);

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
    int offset = sizeof_t(aloop->expr->t_type);
    vector_append(fn->code, "mov rax, 0\n");
    for (int i = 0; i < aloop->exprs->size; i++) {
      aloop_code = safe_alloc(BUFSIZ);
      int mul_val = offset + i * 8 + aloop_info->rank * 8;
      int add_val = offset + i * 8;
      sprintf(aloop_code, "imul rax, [rsp + %d]\nadd rax, [rsp + %d]\n",
              mul_val, add_val);
      vector_append(fn->code, aloop_code);
    }

    aloop_code = safe_alloc(BUFSIZ);
    sprintf(aloop_code, "imul rax, %ld\nadd rax, [rsp + %d]\n",
            sizeof_t(aloop_info->type),
            offset + aloop->exprs->size * 8 + aloop_info->rank * 8);
    vector_append(fn->code, aloop_code);

    // Copy data
    stack_copy(fn, aloop_info->type, "rsp", "rax");
    stack_pop(fn, NULL);

    // Incr bounds
    aloop_code = safe_alloc(BUFSIZ);
    sprintf(aloop_code, "add qword [rsp + %d], 1\n",
            (aloop->exprs->size - 1) * 8);
    vector_append(fn->code, aloop_code);

    for (int i = aloop->vars->size - 1; i >= 0; i--) {
      aloop_code = safe_alloc(BUFSIZ);
      sprintf(aloop_code, "mov rax, [rsp + %d]\ncmp rax, [rsp + %d]\njl %s\n",
              i * 8, (i + aloop->exprs->size) * 8, aloop_body);
      vector_append(fn->code, aloop_code);

      if (i != 0) {
        aloop_code = safe_alloc(BUFSIZ);
        sprintf(aloop_code,
                "mov qword [rsp + %d], 0\nadd qword [rsp + %d], 1\n", i * 8,
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
    break;
  default:
    ir_error("EXPR is not implemented yet");
  }
}
