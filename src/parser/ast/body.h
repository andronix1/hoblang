#pragma once

#include <stdarg.h>
#include "stmts/var.h"
#include "stmts/if_else.h"
#include "stmts/return.h"
#include "stmts/assign.h"

typedef enum {
	AST_STMT_IF,
	AST_STMT_VAR,
	AST_STMT_FUNC_CALL,
	AST_STMT_RETURN,
	AST_STMT_ASSIGN
} AstStmtType;

typedef struct {
	AstStmtType type;
	union {
		AstVar var;
		AstIfElse if_else;
		AstFuncCall func_call;
		AstReturn ret;
		AstAssign assign;
	};
} AstStmt;

typedef struct _AstBody {
	AstStmt *stmts;
} AstBody;

void print_ast_body(FILE *stream, va_list *list);
