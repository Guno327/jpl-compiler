#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>
#include "token.h"

typedef struct {
  Token** data;
  size_t size;
  size_t capacity;
} Vector;

void vector_init(Vector* v, size_t capacity);
void vector_append(Vector* v, Token* item);
void vector_free(Vector* v);

#endif
