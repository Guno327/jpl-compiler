#include "safe.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *safe_strcat(char *lhs, const char *rhs) {
  long new_len = strlen(lhs) + strlen(rhs) + 1;
  lhs = safe_realloc_str(lhs, new_len);
  return strcat(lhs, rhs);
}

void *safe_alloc(long size) {
  void *ret = malloc(size);
  memset(ret, 0, size);
  return ret;
}

char *safe_realloc_str(char *ptr, long size) {
  char *tmp = NULL;
  while (tmp == NULL)
    tmp = realloc(ptr, size);
  return ptr = tmp;
}
