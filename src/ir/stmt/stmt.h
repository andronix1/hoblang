#pragma once

/*
    AST_STMT_IF,
    AST_STMT_WHILE,
*/

#include "ir/stmt/code.h"
#include "ir/stmt/expr.h"

typedef enum {
    IR_STMT_EXPR,
    IR_STMT_STORE,
    IR_STMT_RET,
    IR_STMT_RET_VOID,
} IrStmtKind;

typedef struct {
    IrExpr value;
} IrStmtRet;

typedef struct {
    IrExpr lvalue, rvalue;
} IrStmtStore;

typedef struct {
    IrExpr cond;
    IrCode *code;
} IrStmtCondJmpBlock;

typedef struct {
    IrStmtCondJmpBlock *conds;
    IrCode *else_code;
} IrStmtCondJmp;

typedef struct IrStmt {
    IrStmtKind kind;

    union {
        IrExpr expr;
        IrStmtStore store;
        IrStmtRet ret;
    };
} IrStmt;
