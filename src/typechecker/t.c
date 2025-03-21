#include "t.h"
#include "vector_get.h"
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

size_t sizeof_t(t *type) {
  switch (type->type) {
  case INT_T:
    return 8;
  case FLOAT_T:
    return 8;
  case BOOL_T:
    return 8;
  case VOID_T:
    return 8;
  case STRUCT_T:;
    struct_info *sinfo = (struct_info *)type->info;
    size_t s_size = 0;
    for (int i = 0; i < sinfo->ts->size; i++) {
      t *cur_t = vector_get_t(sinfo->ts, i);
      s_size += sizeof_t(cur_t);
    }
    return s_size;
  case ARRAY_T:;
    array_info *ainfo = (array_info *)type->info;
    return 8 + ainfo->rank * 8;
  case FN_T:
    return 0;
  }
  return 0;
}
