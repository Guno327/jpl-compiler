#include <stdbool.h>
#include <stddef.h>
#ifndef VECTOR_H
#define VECTOR_H

typedef enum {
  TOKENVECTOR,
  CMDVECTOR,
  EXPRVECTOR,
  LVALUEVECTOR,
  TYPEVECTOR,
  STMTVECTOR,
  STRVECTOR,
  BINDINGVECTOR,
  STRUCTINFOVECTOR,
  ARRAYINFOVECTOR,
  FNINFOVECTOR,
  VARINFOVECTOR,
  TVECTOR,
} vector_t;

typedef struct {
  vector_t type;
  void **data;
  size_t size;
  size_t capacity;
} vector;

void vector_init(vector *v, size_t capacity, vector_t type);
void vector_append(vector *v, void *item);

#endif
