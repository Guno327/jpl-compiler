#include "vector.h"
#ifndef T_H
#define T_H

typedef enum { INT_T, FLOAT_T, VOID_T, BOOL_T, ARRAY_T, STRUCT_T, FN_T } t_type;
typedef struct {
  t_type type;
  void *info;
} t;

typedef struct {
  char *name;
  vector *vars;
  vector *ts;
} struct_info;

#endif
