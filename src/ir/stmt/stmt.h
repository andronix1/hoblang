#pragma once

#include "ir/api/local.h"
#include "ir/api/loops.h"
#include "ir/stmt/code.h"
#include "ir/stmt/expr.h"

typedef enum {
    IR_STMT_EXPR,
    IR_STMT_STORE,
    IR_STMT_COND_JMP,
    IR_STMT_RET,
    IR_STMT_RET_VOID,
    IR_STMT_INIT_FINAL,
    IR_STMT_DECL_VAR,
    IR_STMT_LOOP,
    IR_STMT_BREAK,
    IR_STMT_CONTINUE,
} IrStmtKind;

typedef struct {
    IrExpr value;
} IrStmtRet;

typedef struct {
    IrLocalId id;
    IrExpr value;
} IrStmtInitFinal;

typedef struct {
    IrExpr lvalue, rvalue;
} IrStmtStore;

typedef struct {
    IrExpr cond;
    IrCode *code;
} IrStmtCondJmpBlock;

static inline IrStmtCondJmpBlock ir_stmt_cond_jmp_block(IrExpr cond, IrCode *code) {
    IrStmtCondJmpBlock block = {
        .cond = cond,
        .code = code
    };
    return block;
}

typedef struct {
    IrStmtCondJmpBlock *conds;
    IrCode *else_code;

    IrCodeFlow flow;
} IrStmtCondJmp;

typedef struct {
    IrLoopId id;
    IrCode *code;
} IrStmtLoop;

typedef struct {
    IrLoopId id;
} IrStmtLoopFlow;

typedef struct IrStmt {
    IrStmtKind kind;

    union {
        IrExpr expr;
        IrStmtStore store;
        IrStmtCondJmp cond_jmp;
        IrStmtRet ret;
        IrStmtInitFinal init_final;
        IrStmtLoop loop;
        IrStmtLoopFlow break_loop;
        IrStmtLoopFlow continue_loop;
        IrLocalId var_id;
    };
} IrStmt;

IrStmt *ir_stmt_new_expr(Mempool *mempool, IrExpr expr);
IrStmt *ir_stmt_new_loop(Mempool *mempool, IrLoopId id, IrCode *code);
IrStmt *ir_stmt_new_break(Mempool *mempool, IrLoopId id);
IrStmt *ir_stmt_new_continue(Mempool *mempool, IrLoopId id);
IrStmt *ir_stmt_new_store(Mempool *mempool, IrExpr lvalue, IrExpr rvalue);
IrStmt *ir_stmt_new_decl_var(Mempool *mempool, IrLocalId id);
IrStmt *ir_stmt_new_cond_jmp(Mempool *mempool, IrStmtCondJmpBlock *conds, IrCode *else_code);
IrStmt *ir_stmt_new_ret(Mempool *mempool, IrExpr value);
IrStmt *ir_stmt_new_ret_void(Mempool *mempool);
IrStmt *ir_stmt_new_init_final(Mempool *mempool, IrLocalId id, IrExpr expr);
