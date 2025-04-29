#include "asm_ir.h"
#include "compiler_error.h"
#include "safe.h"
#include <stdio.h>

void stmt_asmgen(asm_prog *prog, asm_fn *fn, stmt *s) {
  switch (s->type) {
  case LETSTMT:;
    let_stmt *ls = (let_stmt *)s->node;
    let_asmgen(prog, fn, ls, true);
    break;
  case RETURNSTMT:;
    return_stmt *rs = (return_stmt *)s->node;
    expr_asmgen(prog, fn, rs->expr);

    switch (rs->expr->t_type->type) {
    case FLOAT_T:
      stack_pop(prog, fn, "xmm0");
      break;
    case INT_T:
    case BOOL_T:
    case VOID_T:
      stack_pop(prog, fn, "rax");
      break;
    case STRUCT_T:
    case ARRAY_T:;
      char *code = safe_alloc(BUFSIZ);
      sprintf(code, "mov rax, [rbp - %ld]\n", fn->call->ret_pos);
      vector_append(fn->code, code);
      stack_copy(prog, fn, rs->expr->t_type, "rsp", "rax");
      break;
    default:
      ir_error("T not implemented in return");
    }
    char *rs_code = safe_alloc(BUFSIZ);
    sprintf(rs_code, "add rsp, %ld\npop rbp\nret\n", fn->stk->size);
    vector_append(fn->code, rs_code);
    break;
  case ASSERTSTMT:;
    assert_stmt *as = (assert_stmt *)s->node;
    expr_asmgen(prog, fn, as->expr);
    stack_pop(prog, fn, "rax");
    vector_append(fn->code, "cmp rax, 0\n");
    char *as_str = safe_replace(as->str, '\"', '\0');
    assert_asmgen(prog, fn, "jne", as_str);
    break;
  default:
    ir_error("STMT not implemented");
  }
}
