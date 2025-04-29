#include "vector.h"
#ifndef T_H
#define T_H
struct ctx;

typedef enum {
  INT_T,
  FLOAT_T,
  VOID_T,
  BOOL_T,
  ARRAY_T,
  STRUCT_T,
  FN_T,
  PAD_T,
} t_type;
typedef struct {
  t_type type;
  void *info;
} t;

typedef struct {
  char *name;
  vector *vars;
  vector *ts;
} struct_info;

typedef struct {
  char *name;
  t *type;
  long rank;
} array_info;

typedef struct {
  char *name;
  vector *args;
  t *ret;
  void *dec;
  struct ctx *ctx;
} fn_info;

typedef struct {
  char *name;
  t *t;
} var_info;

bool t_eq(t *lhs, t *rhs);
#endif
