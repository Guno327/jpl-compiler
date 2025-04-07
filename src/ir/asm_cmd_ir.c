#include "asm_ir.h"
#include "compiler_error.h"
#include "safe.h"
#include "typecheck.h"
#include "vector.h"
#include "vector_get.h"
#include <stdio.h>
#include <string.h>

char *int_registers[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
char *float_registers[] = {"xmm0", "xmm1", "xmm2", "xmm3", "xmm4",
                           "xmm5", "xmm6", "xmm7", "xmm8"};

bool is_int_reg(char *reg) {
  for (long i = 0; i < 6; i++) {
    if (!strcmp(reg, int_registers[i]))
      return true;
  }
  return false;
}

bool is_float_reg(char *reg) {
  for (long i = 0; i < 9; i++) {
    if (!strcmp(reg, float_registers[i]))
      return true;
  }
  return false;
}

void cmd_asmgen(asm_prog *prog, asm_fn *fn, cmd *c) {
  switch (c->type) {
  case SHOWCMD:;
    show_cmd *sc = (show_cmd *)c->node;
    stack_align(fn, 16 - sizeof_t(sc->expr->t_type));
    expr_asmgen(prog, fn, sc->expr);

    char *sc_type = t_to_str(sc->expr->t_type);
    char *sc_val = safe_alloc(strlen(sc_type) + BUFSIZ);
    sprintf(sc_val, "db `%s`, 0", sc_type);
    free(sc_type);
    char *sc_const = genconst(prog, sc_val);

    char *sc_code = safe_alloc(1);
    sc_code = safe_strcat(sc_code, "lea rdi, [rel ");
    sc_code = safe_strcat(sc_code, sc_const);
    sc_code = safe_strcat(sc_code, "]\n");

    sc_code = safe_strcat(sc_code, "lea rsi, [rsp]\n");
    sc_code = safe_strcat(sc_code, "call _show\n");
    vector_append(fn->code, sc_code);

    t *sc_pop = stack_pop(fn, NULL);
    if (!t_eq(sc_pop, sc->expr->t_type)) {
      char *msg = safe_alloc(BUFSIZ);
      sprintf(msg, "Expected stack '%s' got '%s'", t_to_str(sc->expr->t_type),
              t_to_str(sc_pop));
      ir_error(msg);
    }
    stack_unalign(fn);
    break;
  case LETCMD:;
    let_cmd *lc = (let_cmd *)c->node;
    let_asmgen(prog, fn, lc, false);
    break;
  case FNCMD:;
    fn_cmd *fc = (fn_cmd *)c->node;
    asm_fn *fc_fn = safe_alloc(sizeof(asm_fn));
    fc_fn->stk = safe_alloc(sizeof(stack));
    fc_fn->code = safe_alloc(sizeof(vector));
    fc_fn->name = fc->var;
    vector_init(fc_fn->code, 8, STRVECTOR);

    fc_fn->stk->fn = fc_fn;
    fc_fn->stk->size = 0;
    fc_fn->stk->names = safe_alloc(sizeof(vector));
    fc_fn->stk->shadow = safe_alloc(sizeof(vector));
    fc_fn->stk->positions = safe_alloc(sizeof(vector));
    vector_init(fc_fn->stk->names, 8, STRVECTOR);
    vector_init(fc_fn->stk->shadow, 8, TVECTOR);
    vector_init(fc_fn->stk->positions, 8, NUMVECTOR);

    fc_fn->call = safe_alloc(sizeof(call_conv));
    fc_fn->call->ret = safe_alloc(BUFSIZ);
    fc_fn->call->args = safe_alloc(sizeof(vector));
    fc_fn->call->types = safe_alloc(sizeof(vector));
    fc_fn->call->lvals = safe_alloc(sizeof(vector));
    fc_fn->call->stk_size = 0;
    vector_init(fc_fn->call->args, fc->binds->size, STRVECTOR);
    vector_init(fc_fn->call->types, fc->binds->size, TYPEVECTOR);
    vector_init(fc_fn->call->lvals, fc->binds->size, LVALUEVECTOR);

    // Prelude
    vector_append(fc_fn->code, "push rbp\nmov rbp, rsp\n");

    // If stack ret
    if (fc->type->type == ARRAYTYPE) {
      stack_push(fc_fn, "rdi");
      t int_t = {INT_T, NULL};
      stack_rechar(fc_fn, &int_t, 1);
      fc_fn->call->ret_pos = 8;
    }
    fc_fn->call->ret_t = type_to_t(fc->type);

    // Build calling convention
    long int_cnt = 0;
    long float_cnt = 0;
    long stk_offset = fc_fn->stk->size;
    for (long i = 0; i < fc->binds->size; i++) {
      binding *cur_bind = vector_get_binding(fc->binds, i);
      vector_append(fc_fn->call->types, cur_bind->type);
      vector_append(fc_fn->call->lvals, cur_bind->lval);
      switch (cur_bind->type->type) {
      case FLOATTYPE:
        if (float_cnt < 9) {
          vector_append(fc_fn->call->args, float_registers[float_cnt]);
          float_cnt += 1;
          break;
        }
      case INTTYPE:
      case BOOLTYPE:
      case VOIDTYPE:
        if (int_cnt < 6 && cur_bind->type->type != FLOATTYPE) {
          vector_append(fc_fn->call->args, int_registers[int_cnt]);
          int_cnt += 1;
          break;
        }
      case ARRAYTYPE:;
        // Fall through if int_cnt/float_cnt exceeds regs
        long arg_size = sizeof_t(type_to_t(cur_bind->type));
        char *arg_offset = safe_alloc(BUFSIZ);
        sprintf(arg_offset, "%ld", stk_offset);
        stk_offset -= arg_size;
        vector_append(fc_fn->call->args, arg_offset);
        fc_fn->call->stk_size += arg_size;
        break;
      default:;
        ir_error("Type not implemented");
      }
    }

    switch (fc->type->type) {
    case FLOATTYPE:
      fc_fn->call->ret = "xmm0";
      break;
    case INTTYPE:
    case BOOLTYPE:
    case VOIDTYPE:
      fc_fn->call->ret = "rax";
      break;
    case ARRAYTYPE:
      fc_fn->call->ret = "rbp - 8";
      break;
    default:;
      ir_error("Type not implemented");
    }

    // Args
    for (long i = 0; i < fc_fn->call->args->size; i++) {
      lval *cur_lval = vector_get_lvalue(fc_fn->call->lvals, i);
      char *cur = vector_get_str(fc_fn->call->args, i);

      if (is_int_reg(cur)) {
        stack_push(fc_fn, cur);
        t int_t = {INT_T, NULL};
        stack_rechar(fc_fn, &int_t, 1);
        push_lval(fc_fn, cur_lval, fc_fn->stk->size);
      } else if (is_float_reg(cur)) {
        stack_push(fc_fn, cur);
        t float_t = {FLOAT_T, NULL};
        stack_rechar(fc_fn, &float_t, 1);
        push_lval(fc_fn, cur_lval, fc_fn->stk->size);
      } else {
        long offset = -1 * (fc_fn->stk->size - strtol(cur, NULL, 10) + 16);
        push_lval(fc_fn, cur_lval, offset);
      }
    }

    // Stmts
    bool found_ret = false;
    for (long i = 0; i < fc->stmts->size; i++) {
      stmt *cur_stmt = vector_get_stmt(fc->stmts, i);
      stmt_asmgen(prog, fc_fn, cur_stmt);

      if (cur_stmt->type == RETURNSTMT)
        found_ret = true;
    }

    // Implicit return
    if (!found_ret && fc_fn->stk->size > 0) {
      char *code = safe_alloc(BUFSIZ);
      sprintf(code, "add rsp, %ld\npop rbp\nret\n", fc_fn->stk->size);
      vector_append(fc_fn->code, code);
    }

    vector_append(prog->fns, fc_fn);
    break;
  default:;
    char *msg = safe_alloc(BUFSIZ);
    sprintf(msg, "CMD not implemented yet");
    ir_error(msg);
  }
}
