#pragma once

#include "hir/api/expr.h"

typedef enum {
    HIR_STMT_EXPR,
    HIR_STMT_STORE,
    HIR_STMT_COND_JMP,
    HIR_STMT_RET,
    HIR_STMT_RET_VOID,
    HIR_STMT_INIT_FINAL,
    HIR_STMT_DECL_VAR,
    HIR_STMT_LOOP,
    HIR_STMT_BREAK,
    HIR_STMT_CONTINUE,
} HirStmtKind;

typedef struct {
    HirExpr value;
} HirStmtRet;

typedef struct {
    HirLocalId id;
    HirExpr value;
} HirStmtInitFinal;

typedef struct {
    HirExpr lvalue, rvalue;
} HirStmtStore;

typedef struct {
    HirExpr cond;
    HirCode *code;
} HirStmtCondJmpBlock;

static inline HirStmtCondJmpBlock hir_stmt_cond_jmp_block(HirExpr cond, HirCode *code) {
    HirStmtCondJmpBlock block = {
        .cond = cond,
        .code = code
    };
    return block;
}

typedef struct {
    HirStmtCondJmpBlock *conds;
    HirCode *else_code;

    bool breaks;
} HirStmtCondJmp;

typedef struct {
    HirLoopId id;
    HirCode *code;
} HirStmtLoop;

typedef struct {
    HirLoopId id;
} HirStmtLoopFlow;

typedef struct HirStmt {
    HirStmtKind kind;

    union {
        HirExpr expr;
        HirStmtStore store;
        HirStmtCondJmp cond_jmp;
        HirStmtRet ret;
        HirStmtInitFinal init_final;
        HirStmtLoop loop;
        HirStmtLoopFlow break_loop;
        HirStmtLoopFlow continue_loop;
        HirLocalId var_id;
    };
} HirStmt;

typedef struct HirCode {
    HirStmt *stmts;
    bool breaks;
} HirCode;

HirCode *hir_code_new(Mempool *mempool, HirStmt *stmts);

static inline HirStmt hir_stmt_new_expr(HirExpr expr) {
    HirStmt stmt = {
        .kind = HIR_STMT_EXPR,
        .expr = expr
    };
    return stmt;
}

static inline HirStmt hir_stmt_new_loop(HirLoopId id, HirCode *code) {
    HirStmt stmt = {
        .kind = HIR_STMT_LOOP,
        .loop = {
            .code = code,
            .id = id,
        }
    };
    return stmt;
}

static inline HirStmt hir_stmt_new_break(HirLoopId id) {
    HirStmt stmt = {
        .kind = HIR_STMT_BREAK,
        .break_loop.id = id
    };
    return stmt;
}

static inline HirStmt hir_stmt_new_continue(HirLoopId id) {
    HirStmt stmt = {
        .kind = HIR_STMT_CONTINUE,
        .continue_loop.id = id
    };
    return stmt;
}

static inline HirStmt hir_stmt_new_store(HirExpr lvalue, HirExpr rvalue) {
    HirStmt stmt = {
        .kind = HIR_STMT_STORE,
        .store = {
            .lvalue = lvalue,
            .rvalue = rvalue
        }
    };
    return stmt;
}

static inline HirStmt hir_stmt_new_decl_var(HirLocalId id) {
    HirStmt stmt = {
        .kind = HIR_STMT_DECL_VAR,
        .var_id = id
    };
    return stmt;
}

static inline HirStmt hir_stmt_new_cond_jmp(HirStmtCondJmpBlock *conds, HirCode *else_code)  {
    HirStmt stmt = {
        .kind = HIR_STMT_COND_JMP,
        .cond_jmp = {
            .conds = conds,
            .else_code = else_code
        }
    };
    return stmt;
}

static inline HirStmt hir_stmt_new_ret(HirExpr value) {
    HirStmt stmt = {
        .kind = HIR_STMT_RET,
        .ret.value = value
    };
    return stmt;
}

static inline HirStmt hir_stmt_new_ret_void() {
    HirStmt stmt = { .kind = HIR_STMT_RET_VOID };
    return stmt;
}

static inline HirStmt hir_stmt_new_init_final(HirLocalId id, HirExpr expr) {
    HirStmt stmt = {
        .kind = HIR_STMT_INIT_FINAL,
        .init_final = {
            .id = id,
            .value = expr,
        }
    };
    return stmt;
}
