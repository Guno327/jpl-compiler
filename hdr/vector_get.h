#include "asm_ir.h"
#include "ast.h"
#include "c_ir.h"
#include "t.h"
#include "token.h"
#include "vector.h"
#ifndef VECTOR_GET_H
#define VECTOR_GET_H

token *vector_get_token(vector *v, long idx);
cmd *vector_get_cmd(vector *v, long idx);
expr *vector_get_expr(vector *v, long idx);
lval *vector_get_lvalue(vector *v, long idx);
type *vector_get_type(vector *v, long idx);
stmt *vector_get_stmt(vector *v, long idx);
char *vector_get_str(vector *v, long idx);
binding *vector_get_binding(vector *v, long idx);
struct_info *vector_get_struct_info(vector *v, long idx);
array_info *vector_get_array_info(vector *v, long idx);
fn_info *vector_get_fn_info(vector *v, long idx);
var_info *vector_get_var_info(vector *v, long idx);
t *vector_get_t(vector *v, long idx);
c_fn *vector_get_c_fn(vector *v, long idx);
c_struct *vector_get_c_struct(vector *v, long idx);
asm_fn *vector_get_asm_fn(vector *v, long idx);
long vector_get_num(vector *v, long idx);

bool vector_contains_str(vector *v, char *str);
#endif
