#include "vector.h"
#include <stdlib.h>
#include <stdio.h>

void vector_init(Vector *v, size_t capacity, VectorType type) {
  switch (type){
    case TOKEN:
      v->data = malloc(capacity * sizeof(Token));
      break;
    case CMD:
      v->data = malloc(capacity * sizeof(Cmd));
      break;
    default:
      return;
  }

  v->type = type; 
  v->size = 0;
  v->capacity = capacity;
}

void vector_append(Vector *v, void *item) {
  if (v->size == v->capacity) {
    v->capacity *= 2;
    switch (v->type){
      case TOKEN:
        v->data = realloc(v->data, v->capacity * sizeof(Token));
        break;
      case CMD:
        v->data = realloc(v->data, v->capacity * sizeof(Cmd));
        break;
      default:
        return;
    }
  }
  v->data[v->size++] = item;
}

void vector_free(Vector *v) {
  for (int i = 0; i < v->size; i++)
    free(v->data[i]);
  free(v);
}

Token* vector_get_token(Vector* v, int idx){
  if (v->type != TOKEN)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  Token* t = (Token*)v->data[idx];
  return t;
}

Cmd* vector_get_cmd(Vector* v, int idx){
  if (v->type != CMD)
    return NULL;
  if (idx < 0 || idx > v->size - 1)
    return NULL;
  Cmd* c = (Cmd*)v->data[idx];
  return c;
}
