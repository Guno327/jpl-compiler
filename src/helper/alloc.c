#include "alloc.h"
#include <string.h>

void *alloc(size_t size) {
  void *ret = malloc(size);
  memset(ret, 0, size);
  return ret;
}
