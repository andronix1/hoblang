#include "stmt.h"

#define STMT_CONSTRUCT(KIND, FIELDS) MEMPOOL_CONSTRUCT(IrStmt, out->kind = KIND; FIELDS)

IrStmt *ir_stmt_new_expr(Mempool *mempool, IrExpr expr)
    STMT_CONSTRUCT(IR_STMT_EXPR, out->expr = expr)

IrStmt *ir_stmt_new_store(Mempool *mempool, IrExpr lvalue, IrExpr rvalue)
    STMT_CONSTRUCT(IR_STMT_STORE,
        out->store.lvalue = lvalue;
        out->store.rvalue = rvalue;
    )

IrStmt *ir_stmt_new_cond_jmp(Mempool *mempool, IrStmtCondJmpBlock *conds, IrCode *else_code) 
    STMT_CONSTRUCT(IR_STMT_COND_JMP,
        out->cond_jmp.conds = conds;
        out->cond_jmp.else_code = else_code;
    )

IrStmt *ir_stmt_new_ret(Mempool *mempool, IrExpr value)
    STMT_CONSTRUCT(IR_STMT_RET, out->ret.value = value)

IrStmt *ir_stmt_new_ret_void(Mempool *mempool)
    STMT_CONSTRUCT(IR_STMT_RET_VOID,)

IrStmt *ir_stmt_new_init_final(Mempool *mempool, IrLocalId id, IrExpr expr)
    STMT_CONSTRUCT(IR_STMT_INIT_FINAL,
        out->init_final.id = id;
        out->init_final.value = expr;
    )
