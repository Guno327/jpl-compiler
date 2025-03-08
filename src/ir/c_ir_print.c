#include "c_ir.h"
#include "compiler_error.h"
#include "safe.h"
#include "typecheck.h"
#include "vector.h"
#include "vector_get.h"

char *c_prog_to_str(c_prog *prog) {
  char *result = safe_alloc(1);

  // Header
  result = safe_strcat(result, C_HDR);

  // Structs
  for (int i = 0; i < prog->structs->size; i++) {
    c_struct *cur = vector_get_c_struct(prog->structs, i);
    result = safe_strcat(result, "typedef struct {\n");

    // Binds
    for (int j = 0; j < cur->fields->size; j++) {
      char *cur_field = vector_get_str(cur->fields, j);
      char *cur_type = vector_get_str(cur->types, j);
      result = safe_strcat(result, cur_type);
      result = safe_strcat(result, " ");
      result = safe_strcat(result, cur_field);
      result = safe_strcat(result, ";\n");
    }

    result = safe_strcat(result, "} ");
    result = safe_strcat(result, cur->name);
    result = safe_strcat(result, ";\n\n");
  }

  // Functions (HW9)
  for (int i = 1; i < prog->fns->size; i++) {
  }

  // Main
  c_fn *jpl_main = vector_get_c_fn(prog->fns, 0);
  result = safe_strcat(result, "void jpl_main(struct args args) {\n");
  for (int i = 0; i < jpl_main->code->size; i++) {
    char *block = vector_get_str(jpl_main->code, i);
    result = safe_strcat(result, block);
  }
  result = safe_strcat(result, "}\n\n");

  return result;
}
