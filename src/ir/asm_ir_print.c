#include "asm_ir.h"
#include "compiler_error.h"
#include "safe.h"
#include "typecheck.h"
#include "vector.h"
#include "vector_get.h"

char *asm_prog_to_str(asm_prog *prog) {
  char *result = safe_alloc(1);

  // Prlong globals and externs
  result = safe_strcat(result, ASM_GLOBALS);
  result = safe_strcat(result, ASM_EXTERNS);
  result = safe_strcat(result, "\n");

  // Data section
  result = safe_strcat(result, "section .data\n");
  for (long i = 0; i < prog->const_names->size; i++) {
    char *cur_name = vector_get_str(prog->const_names, i);
    char *cur_val = vector_get_str(prog->const_vals, i);

    result = safe_strcat(result, cur_name);
    result = safe_strcat(result, ": ");
    result = safe_strcat(result, cur_val);
    result = safe_strcat(result, "\n");
  }
  result = safe_strcat(result, "\n");

  // Text section
  result = safe_strcat(result, "section .text\n");
  for (long i = 1; i < prog->fns->size; i++) {
    asm_fn *cur_fn = vector_get_asm_fn(prog->fns, i);

    result = safe_strcat(result, cur_fn->name);
    result = safe_strcat(result, ":\n");

    result = safe_strcat(result, "_");
    result = safe_strcat(result, cur_fn->name);
    result = safe_strcat(result, ":\n");

    for (long j = 0; j < cur_fn->code->size; j++) {
      char *cur_segment = vector_get_str(cur_fn->code, j);
      result = safe_strcat(result, cur_segment);
    }
    result = safe_strcat(result, "\n");
  }

  // jpl_main
  asm_fn *jpl_main = vector_get_asm_fn(prog->fns, 0);
  result = safe_strcat(result, jpl_main->name);
  result = safe_strcat(result, ":\n");

  result = safe_strcat(result, "_");
  result = safe_strcat(result, jpl_main->name);
  result = safe_strcat(result, ":\n");

  for (long j = 0; j < jpl_main->code->size; j++) {
    char *cur_segment = vector_get_str(jpl_main->code, j);
    result = safe_strcat(result, cur_segment);
  }
  result = safe_strcat(result, "\n");

  return result;
}
