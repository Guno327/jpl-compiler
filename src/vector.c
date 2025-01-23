#include "vector.h"
#include <stdlib.h>
#include <stdio.h>

void vector_init(Vector *v, size_t capacity, size_t type_size) {
  v->data = malloc(capacity * type_size);
  v->size = 0;
  v->capacity = capacity;
  v->type_size = type_size;
}

void vector_append(Vector *v, void *item) {
  if (v->size == v->capacity) {
    v->capacity *= 2;
    v->data = realloc(v->data, v->capacity * v->type_size);
  }
  v->data[v->size++] = item;
}

void vector_free(Vector *v) {
  for (int i = 0; i < v->size; i++)
    free(v->data[i]);
  free(v);
}
