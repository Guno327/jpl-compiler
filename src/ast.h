#ifndef AST_H
#define AST_H
#include <stdbool.h>

// Overall types
typedef enum {READCMD, WRITECMD, LETCMD, ASSERTCMD, PRINTCMD, SHOWCMD, TIMECMD} CmdType;
typedef struct {
  int start;
  CmdType type;
  void* node;
} Cmd;

typedef enum {INTEXPR, FLOATEXPR, TRUEEXPR, FALSEEXPR, VAREXPR, ARRAYLITERALEXPR} ExprType;
typedef struct {
  int start;
  ExprType type;
  void* node;
} Expr;

// Special Cases (VAR)
typedef struct{
  int start;
  char* var;
} VarExpr;

typedef struct {
  int start;
  VarExpr* var; 
} VarLValue;

// CMD types
typedef struct {
  int start;
  char* str;
  VarLValue* lvalue;
} ReadCmd;

typedef struct {
  int start;
  Expr* expr;
  char* str;
} WriteCmd;

typedef struct {
  int start;
  VarLValue* lvalue;
  Expr* expr;
} LetCmd;

typedef struct {
  int start;
  Expr* expr;
  char* str;
} AssertCmd;

typedef struct {
  int start;
  char* str;
} PrintCmd;

typedef struct {
  int start;
  Expr* expr;
} ShowCmd;

typedef struct {
  int start;
  Cmd* cmd;
} TimeCmd;

// EXPR types
typedef struct{
  int start;
  int val;
} IntExpr;

typedef struct{
  int start;
  float val;
} FloatExpr;

typedef struct{
  int start;
  bool val;
} TrueExpr;

typedef struct{
  int start;
  bool val;
} FalseExpr;

typedef struct{
  int start;
  Expr** list;
} ArrayLiteralExpr;

//Methods
char* print_sexpression(Cmd** program);
char* print_cmd(Cmd* cmd);
char* print_expr(Expr* cmd);
char* print_lvalue(VarLValue lvalue);

#endif
