#ifndef T_H
#define T_H

typedef enum { INT_T, FLOAT_T, VOID_T, BOOL_T, ARRAY_T, STRUCT_T, FN_T } t_type;
typedef struct {
  t_type type;
  void *info;
} t;

typedef struct {
  t *type;
  int rank;
} array_info;

typedef struct {
  char *name;
  int field_count;
  char **names;
  t **types;
} struct_info;

typedef struct {
  int arg_count;
  t **args;
  t *ret;
} fn_info;

#endif
