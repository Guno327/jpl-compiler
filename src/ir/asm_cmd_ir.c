#include "asm_ir.h"
#include "compiler_error.h"
#include "safe.h"
#include "typecheck.h"
#include "vector.h"
#include "vector_get.h"
#include <stdio.h>
#include <string.h>

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
    stack_align(fn, 16 + sizeof_t(sc->expr->t_type));
    expr_asmgen(prog, fn, sc->expr);

    char *sc_type = genshowt(sc->expr->t_type);
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
    long int_cnt = 0;
    if (fc->type->type == ARRAYTYPE) {
      stack_push(fc_fn, "rdi");
      int_cnt += 1;
      t int_t = {INT_T, NULL};
      stack_rechar(fc_fn, &int_t, 1);
      fc_fn->call->ret_pos = 8;
    }
    fc_fn->call->ret_t = type_to_t(fc->type);

    // Build calling convention
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
      case ARRAYTYPE:
      case STRUCTTYPE:;
        // Fall through if int_cnt/float_cnt exceeds regs
        long arg_size = sizeof_t(type_to_t(cur_bind->type));
        char *arg_offset = safe_alloc(BUFSIZ);
        sprintf(arg_offset, "%ld", stk_offset);
        stk_offset -= arg_size;
        vector_append(fc_fn->call->args, arg_offset);
        fc_fn->call->stk_size += arg_size;
        break;
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
    case STRUCTTYPE:
      fc_fn->call->ret = "rbp - 8";
      break;
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
  case ASSERTCMD:;
    assert_cmd *asc = (assert_cmd *)c->node;
    expr_asmgen(prog, fn, asc->expr);
    stack_pop(fn, "rax");
    vector_append(fn->code, "cmp rax, 0\n");
    assert_asmgen(prog, fn, "jne", asc->str);
    break;
  case READCMD:;
    read_cmd *rc = (read_cmd *)c->node;

    array_info *rgba_2d_info = safe_alloc(sizeof(array_info));
    rgba_2d_info->rank = 2;
    rgba_2d_info->type = safe_alloc(sizeof(t));
    rgba_2d_info->type->type = STRUCT_T;
    rgba_2d_info->type->info = struct_lookup(prog, "rgba");

    t *rgba_2d = safe_alloc(sizeof(t));
    rgba_2d->type = ARRAY_T;
    rgba_2d->info = rgba_2d_info;
    stack_alloc(fn, rgba_2d);

    free(rgba_2d_info->type);
    free(rgba_2d_info);
    free(rgba_2d);

    vector_append(fn->code, "lea rdi, [rsp]\n");
    stack_align(fn, 0);

    char *rc_str = safe_alloc(strlen(rc->str));
    memcpy(rc_str, rc->str + 1, strlen(rc->str) - 2);

    char *rc_val = safe_alloc(1);
    sprintf(rc_val, "db `%s`, 0", rc_str);

    char *rc_const = genconst(prog, rc_val);
    char *rc_code = safe_alloc(BUFSIZ);
    sprintf(rc_code, "lea rsi, [rel %s]\n", rc_const);
    vector_append(fn->code, rc_code);

    vector_append(fn->code, "call _read_image\n");
    stack_unalign(fn);
    push_lval(fn, rc->lval, fn->stk->size);
    break;
  case WRITECMD:;
    write_cmd *wc = (write_cmd *)c->node;
    stack_align(fn, 24); // size of rgba[,]
    expr_asmgen(prog, fn, wc->expr);

    char *wc_const = genconst(prog, wc->str);
    char *wc_code = safe_alloc(BUFSIZ);
    sprintf(wc_code, "lea rdi, [rel %s]\n", wc_const);
    vector_append(fn->code, wc_code);

    vector_append(fn->code, "call _write_image\n");
    stack_free(fn, sizeof_t(wc->expr->t_type));
    stack_unalign(fn);
    break;
  case PRINTCMD:;
    print_cmd *pc = (print_cmd *)c->node;
    char *pc_const = genconst(prog, pc->str);
    char *pc_code = safe_alloc(BUFSIZ);
    sprintf(pc_code, "lea rdi, [rel %s]\n", pc_const);
    vector_append(fn->code, pc_code);

    stack_align(fn, 0);
    vector_append(fn->code, "call _print\n");
    stack_unalign(fn);
    break;

  case TIMECMD:;
    time_cmd *tc = (time_cmd *)c->node;
    get_time(prog, fn);
    long stack_start = fn->stk->size;

    cmd_asmgen(prog, fn, tc->cmd);
    get_time(prog, fn);
    stack_pop(fn, "xmm0");

    char *tc_code = safe_alloc(BUFSIZ);
    sprintf(tc_code, "movsd xmm1, [rsp + %ld]\nsubsd xmm0, xmm1\n",
            fn->stk->size - stack_start);
    vector_append(fn->code, tc_code);

    stack_align(fn, 0);
    vector_append(fn->code, "call _print_time\n");
    stack_unalign(fn);
    break;
  case STRUCTCMD:;
    struct_cmd *stc = (struct_cmd *)c->node;
    struct_info *stc_info = safe_alloc(sizeof(struct_info));
    stc_info->name = stc->var;
    stc_info->vars = stc->vars;

    stc_info->ts = safe_alloc(sizeof(vector));
    vector_init(stc_info->ts, stc->types->size, TVECTOR);
    for (int i = 0; i < stc->types->size; i++) {
      type *cur = vector_get_type(stc->types, i);
      t *cur_t = type_to_t(cur);
      vector_append(stc_info->ts, cur_t);
    }

    vector_append(prog->structs, stc_info);
    break;
  default:;
    char *msg = safe_alloc(BUFSIZ);
    sprintf(msg, "CMD not implemented yet");
    ir_error(msg);
  }
}

void get_time(asm_prog *prog, asm_fn *fn) {
  // Prepare stack
  stack_align(fn, 0);

  // do call
  vector_append(fn->code, "call _get_time\n");

  // free stack
  stack_unalign(fn);

  // push ret
  stack_push(fn, "xmm0");
  t *float_t = safe_alloc(sizeof(t));
  float_t->type = FLOAT_T;
  float_t->info = NULL;
  stack_rechar(fn, float_t, 1);
}
