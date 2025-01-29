#ifndef PARSE_STMT_H
#define PARSE_STMT_H
#include "ast.h"
#include "vector.h"

int parse_stmt(Vector *tokens, int i, Stmt *s);

#endif
