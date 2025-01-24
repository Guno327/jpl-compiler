#include "parser.h"
#include "ast.h"
#include "error.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

CmdVector* parse(TokenVector* tokens){
  CmdVector* program = malloc(sizeof(CmdVector));
  vector_init_cmd(program, BUFSIZ);

  int i = 0;
  for(; i < tokens->size; i++){
    if (peek_token(tokens, i) == NEWLINE)
      continue;
    if (peek_token(tokens, i) == END_OF_FILE)
      break;

    Cmd* c = malloc(sizeof(Cmd));
    memset(c, 0, sizeof(Cmd));
    i  = parse_cmd(tokens, i, c);
    vector_append_cmd(program, c);
    expect_token(tokens, i, NEWLINE);
  }
  return program;
}

int parse_cmd(TokenVector* tokens, int i, Cmd* c){
  c->start = i;
  int type = peek_token(tokens, i);

  // Build sub-node
  // General form: Setup \n\n [single token step \n\n ...] 
  switch (type){
    case READ:;
      ReadCmd* rc = malloc(sizeof(ReadCmd));
      memset(rc, 0, sizeof(ReadCmd));
      rc->start = i;
      i += 1;

      expect_token(tokens, i, IMAGE);
      expect_token(tokens, i + 1, STRING);
      i += 1;
      char* rc_str = vector_get_token(tokens, i)->text;
      rc->str = malloc(strlen(rc_str) + 1);
      memset(rc->str, 0, strlen(rc_str) + 1);
      memcpy(rc->str, rc_str, strlen(rc_str));
      i += 1;

      expect_token(tokens, i, TO);
      expect_token(tokens, i+1, VARIABLE);
      i += 1;
      rc->lvalue = malloc(sizeof(VarLValue));
      memset(rc->lvalue, 0, sizeof(VarLValue));
      i = parse_lvalue(tokens, i, rc->lvalue);
      c->node = rc;
      c->type = READCMD;
      break;
    case WRITE:;
      WriteCmd* wc = malloc(sizeof(WriteCmd));
      memset(wc, 0, sizeof(WriteCmd));
      wc->start = i;
      i += 1;

      expect_token(tokens, i, IMAGE);
      i += 1;
      wc->expr = malloc(sizeof(Expr));
      memset(wc->expr, 0, sizeof(Expr));
      i = parse_expr(tokens, i, wc->expr);
      
      expect_token(tokens, i, TO);
      i += 1;
      expect_token(tokens, i, STRING);
      char* wc_str = vector_get_token(tokens, i)->text;
      wc->str = malloc(strlen(wc_str) + 1);
      memset(wc->str, 0, strlen(wc_str) + 1);
      memcpy(wc->str, wc_str, strlen(wc_str));
      i += 1;
      c->node = wc;
      c->type = WRITECMD;
      break;
    case LET:;
      LetCmd* lc = malloc(sizeof(LetCmd));
      memset(lc, 0, sizeof(LetCmd));
      lc->start = i;
      i += 1;

      lc->lvalue = malloc(sizeof(VarLValue));
      memset(lc->lvalue, 0, sizeof(VarLValue));
      i = parse_lvalue(tokens, i, lc->lvalue);
      
      expect_token(tokens, i, EQUALS);
      i += 1;

      lc->expr = malloc(sizeof(Expr));
      memset(lc->expr, 0, sizeof(Expr));
      i = parse_expr(tokens, i, lc->expr);
      c->node = lc;
      c->type = LETCMD;
      break;
    case ASSERT:;
      AssertCmd* ac = malloc(sizeof(AssertCmd));
      memset(ac, 0, sizeof(AssertCmd));
      ac->start = i;
      i += 1;

      ac->expr = malloc(sizeof(Expr));
      memset(ac->expr, 0, sizeof(Expr));
      i = parse_expr(tokens, i, ac->expr);

      expect_token(tokens, i, COMMA);
      expect_token(tokens, i+1, STRING);
      i += 1;
      char* ac_str = vector_get_token(tokens, i)->text;
      ac->str = malloc(strlen(ac_str) + 1);
      memset(ac->str, 0, strlen(ac_str) + 1);
      memcpy(ac->str, ac_str, strlen(ac_str));
      i += 1;
      c->node = ac;
      c->type = ASSERTCMD;
      break;
    case PRINT:;
      PrintCmd* pc = malloc(sizeof(PrintCmd));
      memset(pc, 0, sizeof(PrintCmd));
      pc->start = i;
      i += 1;

      expect_token(tokens, i, STRING);
      char* pc_str = vector_get_token(tokens, i)->text;
      pc->str = malloc(strlen(pc_str) + 1);
      memset(pc->str, 0, strlen(pc_str) + 1);
      memcpy(pc->str, pc_str, strlen(pc_str));
      i += 1;
      c->node = pc;
      c->type = PRINTCMD;
      break;
    case SHOW:;
      ShowCmd* sc = malloc(sizeof(ShowCmd));
      memset(sc, 0, sizeof(ShowCmd));
      sc->start = i;
      i += 1;

      sc->expr = malloc(sizeof(Expr));
      memset(sc->expr, 0, sizeof(Expr));
      i = parse_expr(tokens, i, sc->expr);
      c->node = sc;
      c->type = SHOWCMD;
      break;
    case TIME:;
      TimeCmd* tc = malloc(sizeof(TimeCmd));
      memset(tc, 0, sizeof(TimeCmd));
      tc->start = i;
      i += 1;

      tc->cmd = malloc(sizeof(Cmd));
      memset(tc->cmd, 0, sizeof(Cmd));
      i = parse_cmd(tokens, i, tc->cmd);
      c->node = tc;
      c->type = TIMECMD;
      break;
    default:;
      char* msg = malloc(BUFSIZ);
    sprintf(msg, "Unexpected token '%s' at %d", vector_get_token(tokens, i)->text, i);
      parse_error(msg);
  }

  return i;
}

int parse_expr(TokenVector* tokens, int i, Expr* e){
  e->start = i;
  int type = peek_token(tokens, i);
  
  switch (type){
    case INTVAL:;
      IntExpr* ie = malloc(sizeof(IntExpr));
      memset(ie, 0, sizeof(IntExpr));
      ie->start = i;

      char* ie_str = vector_get_token(tokens, i)->text;
      ie->val = strtol(ie_str, NULL, 10);
      if(errno == ERANGE){
        char* msg = malloc(BUFSIZ);
        sprintf(msg, "Int '%s' out of range", ie_str);
        parse_error(msg);
      }

      i += 1;
      e->node = ie;
      e->type = INTEXPR;
      break;
    case FLOATVAL:;
      FloatExpr* fe = malloc(sizeof(FloatExpr));
      memset(fe, 0, sizeof(FloatExpr));
      fe->start = i;

      char* fe_str = vector_get_token(tokens, i)->text;
      fe->val = strtod(fe_str, NULL);
      if(errno == ERANGE){
        char* msg = malloc(BUFSIZ);
        sprintf(msg, "Int '%s' out of range", fe_str);
        parse_error(msg);
      }

      i += 1;
      e->node = fe;
      e->type = FLOATEXPR;
      break;
    case TRUE:;
      TrueExpr* te = malloc(sizeof(TrueExpr));
      memset(te, 0, sizeof(TrueExpr));
      te->start = i;
      e->node = te;
      e->type = TRUEEXPR;
      i += 1;
      break;
    case FALSE:;
      FalseExpr* fae = malloc(sizeof(FalseExpr));
      memset(fae, 0, sizeof(FalseExpr));
      fae->start = i;
      e->node = fae;
      e->type = FALSEEXPR;
      i += 1;
      break;
    case VARIABLE:;
      VarExpr* ve = malloc(sizeof(VarExpr));
      memset(ve, 0, sizeof(VarExpr));
      ve->start = i;
      char* ve_var = vector_get_token(tokens, i)->text;
      ve->var = malloc(strlen(ve_var) + 1);
      memset(ve->var, 0, strlen(ve_var) + 1);
      memcpy(ve->var, ve_var, strlen(ve_var));
      e->node = ve;
      e->type = VAREXPR;
      i += 1;
      break;
    case LSQUARE:;
      ArrayLiteralExpr* ale = malloc(sizeof(ArrayLiteralExpr));
      memset(ale, 0, sizeof(ArrayLiteralExpr));
      i = parse_array(tokens, i, ale);
      e->node = ale;
      e->type = ARRAYLITERALEXPR;
      break;
    default:;
      char* msg = malloc(BUFSIZ);
      sprintf(msg, "Unexpected token '%s' at %d", vector_get_token(tokens, i)->text, i);
      parse_error(msg);
  }

  return i;
}

int parse_lvalue(TokenVector* tokens, int i, VarLValue* v){
  v->start = i;
  expect_token(tokens, i, VARIABLE);
  char* v_var = vector_get_token(tokens, i)->text;
  v->var = malloc(strlen(v_var) + 1);
  memset(v->var, 0, strlen(v_var) + 1);
  memcpy(v->var, v_var, strlen(v_var));
  return i + 1;
}

int parse_array(TokenVector* tokens, int i, ArrayLiteralExpr* a){
  ExprVector* nodes = malloc(sizeof(ExprVector));
  memset(nodes, 0, sizeof(ExprVector));
  vector_init_expr(nodes, BUFSIZ);

  expect_token(tokens, i, LSQUARE);
  i += 1;
  while (i < tokens->size - 1){
    if (peek_token(tokens, i) == RSQUARE){
      a->list = (Expr**)(nodes->data);
      a->list_size = nodes->size;
      i += 1;
      break;
    } 
    else{
      Expr* e = malloc(sizeof(Expr));
      memset(e, 0, sizeof(Expr));
      e->start = i;
      i = parse_expr(tokens, i, e);
      vector_append_expr(nodes, e);

      if (peek_token(tokens, i) == RSQUARE){
        a->list = (Expr**)(nodes->data);
        a->list_size = nodes->size;
        i += 1;
        break;
      }

      expect_token(tokens, i, COMMA);
      i += 1;
    }
  }

  return i;
}

int peek_token(TokenVector* tokens, int idx){
  Token* t = vector_get_token(tokens, idx);
  return t->type;
}

void expect_token(TokenVector* tokens, int idx, int tok_type){
  Token* t = vector_get_token(tokens, idx);
  if (t->type != tok_type){
    char* msg = malloc(BUFSIZ);
    sprintf(msg, "Unexpected token '%s' at %d", vector_get_token(tokens, idx)->text, idx);
    parse_error(msg);
  }
}
