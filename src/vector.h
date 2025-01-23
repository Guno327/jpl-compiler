#ifndef VECTOR_H
#define VECTOR_H

#include "token.h"
#include <stddef.h>

typedef struct {
  void **data;
  size_t type_size;
  size_t size;
  size_t capacity;
} Vector;

void vector_init(Vector* v, size_t capacity, size_t type_size);
void vector_append(Vector* v, void* item);
void vector_free(Vector* v);

#endif
