#include "vector.h"
#include <stdlib.h>

void vector_init(Vector *v, size_t capacity) {
  v->data = malloc(capacity * sizeof(Token));
  v->size = 0;
  v->capacity = capacity;
}

void vector_append(Vector *v, Token *item) {
  if (v->size == v->capacity) {
    v->capacity *= 2;
    v->data = realloc(v->data, v->capacity * sizeof(Token));
  }
  v->data[v->size++] = item;
}

void vector_free(Vector *v) {
  for (int i = 0; i < v->size; i++)
    free(v->data[i]);
  free(v);
}
