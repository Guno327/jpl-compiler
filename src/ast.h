#ifndef AST_H
#define AST_H
#include <stdbool.h>

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
} CmdType;
typedef struct {
  int start;
  CmdType type;
  void *node;
} Cmd;

typedef enum {
  INTEXPR,
  FLOATEXPR,
  TRUEEXPR,
  FALSEEXPR,
  VAREXPR,
  ARRAYLITERALEXPR,
  VOIDEXPR,
  STRUCTLITERALEXPR,
  DOTEXPR,
  ARRAYINDEXEXPR,
  CALLEXPR
} ExprType;
typedef struct {
  int start;
  ExprType type;
  void *node;
} Expr;

typedef enum {
  INTTYPE,
  BOOLTYPE,
  FLOATTYPE,
  ARRAYTYPE,
  VOIDTYPE,
  STRUCTTYPE
} TypeType;
typedef struct {
  int start;
  TypeType type;
  void *node;
} Type;

typedef enum { LETSTMT, ASSERTSTMT, RETURNSTMT } StmtType;
typedef struct {
  int start;
  StmtType type;
  void *node;
} Stmt;

typedef enum { VARLVALUE, ARRAYLVALUE } LValueType;
typedef struct {
  int start;
  LValueType type;
  void *node;
} LValue;

// Binding
typedef struct {
  int start;
  LValue *lval;
  Type *type;
} Binding;

// LValue types
typedef struct {
  int start;
  char *var;
} VarLValue;

typedef struct {
  int start;
  char *var;
  int lvals_size;
  char **lvals;
} ArrayLValue;

// Stmt types
typedef struct {
  int start;
  LValue *lval;
  Expr *expr;
} LetStmt;

typedef struct {
  int start;
  Expr *expr;
  char *str;
} AssertStmt;

typedef struct {
  int start;
  Expr *expr;
} ReturnStmt;

// TYPE types
typedef struct {
  int start;
} IntType;

typedef struct {
  int start;
} BoolType;

typedef struct {
  int start;
} FloatType;

// Array grammar needs some work to be LL1
typedef struct {
  int start;
  int rank;
  void *typearr;
} TypeArr;

typedef struct {
  int start;
  Type *type;
  TypeArr *arr;
} ArrayType;

// Back to standard types
typedef struct {
  int start;
} VoidType;

typedef struct {
  int start;
  char *var;
} StructType;

// CMD types
typedef struct {
  int start;
  char *str;
  LValue *lval;
} ReadCmd;

typedef struct {
  int start;
  Expr *expr;
  char *str;
} WriteCmd;

typedef struct {
  int start;
  LValue *lval;
  Expr *expr;
} LetCmd;

typedef struct {
  int start;
  Expr *expr;
  char *str;
} AssertCmd;

typedef struct {
  int start;
  char *str;
} PrintCmd;

typedef struct {
  int start;
  Expr *expr;
} ShowCmd;

typedef struct {
  int start;
  Cmd *cmd;
} TimeCmd;

typedef struct {
  int start;
  char *var;
  int binds_size;
  Binding **binds;
  int stmts_size;
  Stmt **stmts;
} FnCmd;

typedef struct {
  int start;
  char *var;
  int vars_size;
  char **vars;
  int types_size;
  Type **types;
} StructCmd;

// EXPR types
typedef struct {
  int start;
  char *var;
} VarExpr;

typedef struct {
  int start;
  long val;
} IntExpr;

typedef struct {
  int start;
  double val;
} FloatExpr;

typedef struct {
  int start;
} TrueExpr;

typedef struct {
  int start;
} FalseExpr;

typedef struct {
  int start;
  int exprs_size;
  Expr **exprs;
} ArrayLiteralExpr;

typedef struct {
  int start;
} VoidExpr;

typedef struct {
  int start;
  char *var;
  int exprs_size;
  Expr **exprs;
} StructLiteralExpr;

typedef struct {
  int start;
  Expr *expr;
  char *var;
} DotExpr;

typedef struct {
  int start;
  Expr *expr;
  int exprs_size;
  Expr **exprs;
} ArrayIndexExpr;

typedef struct {
  int start;
  char *var;
  int list_size;
  Expr *list;
} CallExpr;

// Methods
char *print_cmd(Cmd *cmd);
char *print_expr(Expr *cmd);
char *print_lvalue(LValue *lval);

#endif
