#include "t.h"
#include <string.h>

bool t_eq(t *lhs, t *rhs) {
  if (lhs->type != rhs->type)
    return false;

  switch (lhs->type) {
  case FLOAT_T:
  case INT_T:
  case BOOL_T:
  case VOID_T:
    return true;
  case STRUCT_T:;
    struct_info *lhs_struct = (struct_info *)lhs->info;
    struct_info *rhs_struct = (struct_info *)rhs->info;

    if (!strcmp(lhs_struct->name, rhs_struct->name))
      return true;
    break;
  case ARRAY_T:;
    array_info *lhs_array = (array_info *)lhs->info;
    array_info *rhs_array = (array_info *)rhs->info;

    if (lhs_array->rank == rhs_array->rank &&
        t_eq(lhs_array->type, rhs_array->type))
      return true;
    break;
  case FN_T:;
    fn_info *lhs_fn = (fn_info *)lhs->info;
    fn_info *rhs_fn = (fn_info *)rhs->info;

    if (!strcmp(lhs_fn->name, rhs_fn->name))
      return true;
    break;
  }
  return false;
}
