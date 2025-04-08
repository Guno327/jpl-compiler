#include "t.h"
#include <stdbool.h>
#include <stdlib.h>
#ifndef AST_H
#define AST_H

// Overall types
typedef enum {
  READCMD,
  WRITECMD,
  LETCMD,
  ASSERTCMD,
  PRINTCMD,
  SHOWCMD,
  TIMECMD,
  FNCMD,
  STRUCTCMD
} cmd_t;
typedef struct {
  long start;
  cmd_t type;
  void *node;
} cmd;

typedef enum {
  INTEXPR,
  FLOATEXPR,
  TRUEEXPR,
  FALSEEXPR,
  VAREXPR,
  EXPR,
  ARRAYLITERALEXPR,
  VOIDEXPR,
  STRUCTLITERALEXPR,
  DOTEXPR,
  ARRAYINDEXEXPR,
  CALLEXPR,
  UNOPEXPR,
  BINOPEXPR,
  IFEXPR,
  ARRAYLOOPEXPR,
  SUMLOOPEXPR
} expr_t;
typedef enum { NOTOP, NEGOP } u_op;
typedef enum {
  MULTOP,
  DIVOP,
  MODOP,
  ADDOP,
  SUBOP,
  GTOP,
  LTOP,
  GEOP,
  LEOP,
  EQOP,
  NEOP,
  ANDOP,
  OROP
} b_op;
typedef struct {
  long start;
  expr_t type;
  t *t_type;
  void *node;
} expr;

typedef enum {
  INTTYPE,
  BOOLTYPE,
  FLOATTYPE,
  ARRAYTYPE,
  VOIDTYPE,
  STRUCTTYPE
} type_t;
typedef struct {
  long start;
  type_t type;
  void *node;
} type;

typedef enum { LETSTMT, ASSERTSTMT, RETURNSTMT } stmt_t;
typedef struct {
  long start;
  stmt_t type;
  void *node;
} stmt;

typedef enum { VARLVALUE, ARRAYLVALUE } lval_t;
typedef struct {
  long start;
  lval_t type;
  void *node;
} lval;

// binding
typedef struct {
  lval *lval;
  type *type;
} binding;

// lval types
typedef struct {
  long start;
  char *var;
} var_lval;

typedef struct {
  long start;
  char *var;
  vector *vars;
} array_lval;

// stmt types
typedef struct {
  long start;
  lval *lval;
  expr *expr;
} let_stmt;

typedef struct {
  long start;
  expr *expr;
  char *str;
} assert_stmt;

typedef struct {
  long start;
  expr *expr;
} return_stmt;

// TYPE types
typedef struct {
  long start;
} int_type;

typedef struct {
  long start;
} bool_type;

typedef struct {
  long start;
} float_type;

typedef struct {
  long start;
  type *type;
  long rank;
} array_type;

typedef struct {
  long start;
} void_type;

typedef struct {
  long start;
  char *var;
} struct_type;

// CMD types
typedef struct {
  long start;
  char *str;
  lval *lval;
} read_cmd;

typedef struct {
  long start;
  expr *expr;
  char *str;
} write_cmd;

typedef struct {
  long start;
  lval *lval;
  expr *expr;
} let_cmd;

typedef struct {
  long start;
  expr *expr;
  char *str;
} assert_cmd;

typedef struct {
  long start;
  char *str;
} print_cmd;

typedef struct {
  long start;
  expr *expr;
} show_cmd;

typedef struct {
  long start;
  cmd *cmd;
} time_cmd;

typedef struct {
  long start;
  char *var;
  vector *binds;
  type *type;
  vector *stmts;
} fn_cmd;

typedef struct {
  long start;
  char *var;
  vector *vars;
  vector *types;
} struct_cmd;

// EXPR types
typedef struct {
  long start;
  char *var;
} var_expr;

typedef struct {
  long start;
  long val;
} int_expr;

typedef struct {
  long start;
  double val;
  char *val_str;
} float_expr;

typedef struct {
  long start;
} true_expr;

typedef struct {
  long start;
} false_expr;

typedef struct {
  long start;
  vector *exprs;
} array_literal_expr;

typedef struct {
  long start;
} void_expr;

typedef struct {
  long start;
  char *var;
  vector *exprs;
} struct_literal_expr;

typedef struct {
  long start;
  expr *expr;
  char *var;
} dot_expr;

typedef struct {
  long start;
  u_op op;
  expr *rhs;
} unop_expr;

typedef struct {
  long start;
  expr *lhs;
  b_op op;
  expr *rhs;
} binop_expr;

typedef struct {
  long start;
  expr *if_expr;
  expr *then_expr;
  expr *else_expr;
} if_expr;

typedef struct {
  long start;
  vector *vars;
  vector *exprs;
  expr *expr;
} array_loop_expr;

typedef struct {
  long start;
  vector *vars;
  vector *exprs;
  expr *expr;
} sum_loop_expr;

// back to standard exprs
typedef struct {
  long start;
  expr *expr;
  vector *exprs;
} array_index_expr;

typedef struct {
  long start;
  char *var;
  vector *exprs;
} call_expr;

// Methods
char *cmd_to_str(cmd *cmd);
char *expr_to_str(expr *expr);
char *uop_to_str(u_op op);
char *bop_to_str(b_op op);
char *lval_to_str(lval *lval);
char *stmt_to_str(stmt *stmt);
char *type_to_str(type *type);
char *t_to_str(t *t);

typedef enum {
  CMDLIST,
  EXPRLIST,
  LVALUELIST,
  STMTLIST,
  TYPELIST,
  VARLIST
} list_t;
char *vector_to_str(vector *v);

#endif
