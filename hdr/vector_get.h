#include "asm_ir.h"
#include "ast.h"
#include "c_ir.h"
#include "t.h"
#include "token.h"
#include "vector.h"
#ifndef VECTOR_GET_H
#define VECTOR_GET_H

token *vector_get_token(vector *v, int idx);
cmd *vector_get_cmd(vector *v, int idx);
expr *vector_get_expr(vector *v, int idx);
lval *vector_get_lvalue(vector *v, int idx);
type *vector_get_type(vector *v, int idx);
stmt *vector_get_stmt(vector *v, int idx);
char *vector_get_str(vector *v, int idx);
binding *vector_get_binding(vector *v, int idx);
struct_info *vector_get_struct_info(vector *v, int idx);
array_info *vector_get_array_info(vector *v, int idx);
fn_info *vector_get_fn_info(vector *v, int idx);
var_info *vector_get_var_info(vector *v, int idx);
t *vector_get_t(vector *v, int idx);
c_fn *vector_get_c_fn(vector *v, int idx);
c_struct *vector_get_c_struct(vector *v, int idx);
asm_fn *vector_get_asm_fn(vector *v, int idx);
size_t vector_get_num(vector *v, int idx);
#endif
