#include "alloc.h"
#include <string.h>

void *alloc(int size) {
  void *ret = malloc(size);
  memset(ret, 0, size);
  return ret;
}
