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
  CFNVECTOR,
  CSTRUCTVECTOR,
  ASMFNVECTOR,
} vector_t;

typedef struct {
  vector_t type;
  void **data;
  int size;
  int capacity;
} vector;

void vector_init(vector *v, size_t capacity, vector_t type);
void vector_append(vector *v, void *item);

#endif
