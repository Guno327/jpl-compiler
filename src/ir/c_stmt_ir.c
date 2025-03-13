#include "c_ir.h"
#include "compiler_error.h"
#include "safe.h"
#include "typecheck.h"
#include "vector.h"
#include "vector_get.h"
#include <stdio.h>
#include <string.h>

bool stmt_gencode(c_prog *prog, c_fn *fn, stmt *s) {
  switch (s->type) {
  case LETSTMT:;
    let_stmt *ls = (let_stmt *)s->node;
    char *ls_sym = expr_gencode(prog, fn, ls->expr);
    char *ls_var = NULL;
    switch (ls->lval->type) {
    case VARLVALUE:
      ls_var = ((var_lval *)ls->lval->node)->var;
      break;
    case ARRAYLVALUE:
      ls_var = ((array_lval *)ls->lval->node)->var;
      break;
    }
    vector_append(fn->jpl_names, ls_var);
    vector_append(fn->c_names, ls_sym);

    if (ls->lval->type == ARRAYLVALUE) {
      array_lval *alv = (array_lval *)ls->lval->node;
      for (int i = 0; i < alv->vars->size; i++) {
        vector_append(fn->jpl_names, vector_get_str(alv->vars, i));
        char *c_name = safe_alloc(BUFSIZ);
        sprintf(c_name, "%s.d%d", ls_sym, i);
        vector_append(fn->c_names, c_name);
      }
    }
    break;

  case ASSERTSTMT:;
    assert_stmt *as = (assert_stmt *)s->node;
    char *as_sym = expr_gencode(prog, fn, as->expr);

    char *as_code = safe_alloc(1);
    as_code = safe_strcat(as_code, "if (0 != ");
    as_code = safe_strcat(as_code, as_sym);
    as_code = safe_strcat(as_code, ")\n");

    char *pass_jmp = genjmp(prog);
    as_code = safe_strcat(as_code, "goto ");
    as_code = safe_strcat(as_code, pass_jmp);
    as_code = safe_strcat(as_code, ";\n");

    as_code = safe_strcat(as_code, "fail_assertion(");
    as_code = safe_strcat(as_code, as->str);
    as_code = safe_strcat(as_code, ");\n");
    as_code = safe_strcat(as_code, pass_jmp);
    free(pass_jmp);
    as_code = safe_strcat(as_code, ":;\n");

    vector_append(fn->code, as_code);
    break;
  case RETURNSTMT:;
    return_stmt *rs = (return_stmt *)s->node;
    char *rs_sym = expr_gencode(prog, fn, rs->expr);
    char *rs_code = safe_alloc(1);

    rs_code = safe_strcat(rs_code, "return ");
    rs_code = safe_strcat(rs_code, rs_sym);
    rs_code = safe_strcat(rs_code, ";\n");
    vector_append(fn->code, rs_code);
    break;
  }
  bool ret = (s->type == RETURNSTMT);
  return ret;
}
