#include "asm_ir.h"
#include "compiler_error.h"
#include "safe.h"
#include "vector_get.h"
#include <stdio.h>

void expr_asmgen(asm_prog *prog, asm_fn *fn, expr *e) {
  switch (e->type) {
  case FLOATEXPR:;
    float_expr *fe = (float_expr *)e->node;
    char *fe_val = safe_alloc(BUFSIZ);
    sprintf(fe_val, "dq %f", fe->val);
    char *fe_const = genconst(prog, fe_val);
    free(fe_val);

    char *fe_code = safe_alloc(1);
    fe_code = safe_strcat(fe_code, "mov rax, [rel ");
    fe_code = safe_strcat(fe_code, fe_const);
    fe_code = safe_strcat(fe_code, "]\n");
    vector_append(fn->code, fe_code);

    stack_push(fn, "rax", e->t_type);
    break;
  case INTEXPR:;
    int_expr *ie = (int_expr *)e->node;
    char *ie_val = safe_alloc(BUFSIZ);
    sprintf(ie_val, "dq %lu", ie->val);
    char *ie_const = genconst(prog, ie_val);

    char *ie_code = safe_alloc(1);
    ie_code = safe_strcat(ie_code, "mov rax, [rel ");
    ie_code = safe_strcat(ie_code, ie_const);
    ie_code = safe_strcat(ie_code, "]\n");
    vector_append(fn->code, ie_code);

    stack_push(fn, "rax", e->t_type);
    break;
  case TRUEEXPR:
  case FALSEEXPR:;
    char *be_val = safe_alloc(BUFSIZ);
    int be_num = e->type == TRUEEXPR ? 1 : 0;
    sprintf(be_val, "dq %d", be_num);
    char *be_const = genconst(prog, be_val);
    free(be_val);

    char *be_code = safe_alloc(1);
    be_code = safe_strcat(be_code, "mov rax, [rel ");
    be_code = safe_strcat(be_code, be_const);
    be_code = safe_strcat(be_code, "]\n");
    vector_append(fn->code, be_code);

    stack_push(fn, "rax", e->t_type);
    break;
  case UNOPEXPR:;
    unop_expr *ue = (unop_expr *)e->node;
    expr_asmgen(prog, fn, ue->rhs);

    char *ue_code = safe_alloc(1);
    switch (ue->rhs->t_type->type) {
    case FLOAT_T:
      stack_pop(fn, "xmm1");
      ue_code = safe_strcat(ue_code, "pxor xmm0, xmm1\n");
      ue_code = safe_strcat(ue_code, "subsd xmm0, xmm1\n");
      ue_code = safe_strcat(ue_code, "pxor xmm0, xmm1\n");
      stack_push(fn, "xmm0", ue->rhs->t_type);
      break;
    case INT_T:
      stack_pop(fn, "rax");
      ue_code = safe_strcat(ue_code, "neg rax\n");
      stack_push(fn, "rax", ue->rhs->t_type);
      break;
    case BOOL_T:
      stack_pop(fn, "rax");
      ue_code = safe_strcat(ue_code, "xor rax, 1\n");
      stack_push(fn, "rax", ue->rhs->t_type);
      break;
    default:
      ir_error("Invalid type in UNOP");
    }
    vector_append(fn->code, ue_code);
    break;
  case BINOPEXPR:;
    binop_expr *boe = (binop_expr *)e->node;
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
        assert_asmgen(prog, fn, "divide by 0");

        boe_code = safe_strcat(boe_code, "cqo\nidiv r10\n");
      }
      break;
    case MODOP:
      if (float_mode)
        boe_code = safe_strcat(boe_code, "call _fmod\n");
      else {
        boe_code = safe_strcat(boe_code, "cmp r10, 0\n");

        vector_append(fn->code, boe_code);
        boe_code = safe_alloc(1);
        assert_asmgen(prog, fn, "divide by 0");

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
            boe_code, "cmpnesd xmm0, xmm1\nmovq rax, xmm0\nand rax, 1\n");
      else
        boe_code =
            safe_strcat(boe_code, "cmp rax, r10\nsetne al\nand rax, 1\n");
      break;
    case ANDOP:
    case OROP:
      ir_error("BINOPEXPR not yet implemented");
    }
    vector_append(fn->code, boe_code);

    if (float_mode)
      stack_push(fn, "xmm0", e->t_type);
    else
      stack_push(fn, "rax", e->t_type);
    break;
  case ARRAYLITERALEXPR:
  default:
    ir_error("EXPR is not implemented yet");
  }
}
