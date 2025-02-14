#include "ast.h"
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
t *vector_get_t(vector *v, int idx);

#endif
