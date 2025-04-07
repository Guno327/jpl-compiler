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
  NUMVECTOR,
} vector_t;

typedef struct {
  vector_t type;
  void **data;
  long size;
  long capacity;
} vector;

void vector_init(vector *v, long capacity, vector_t type);
void vector_append(vector *v, void *item);

#endif
