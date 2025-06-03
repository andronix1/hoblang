#include "code.h"
#include "core/assert.h"
#include "ir/stmt/code.h"
#include "ir/stmt/stmt.h"
#include "llvm/module/emit/expr.h"
#include "llvm/module/module.h"
#include <llvm-c/Core.h>

static void llvm_emit_stmt(LlvmModule *module, IrStmt *stmt) {
    switch (stmt->kind) {
        case IR_STMT_EXPR:
            llvm_emit_expr(module, &stmt->expr, false);
            break;
        case IR_STMT_RET:
            LLVMBuildRet(module->builder, llvm_emit_expr(module, &stmt->ret.value, true));
            break;
        case IR_STMT_RET_VOID:
            LLVMBuildRetVoid(module->builder);
            break;
        case IR_STMT_COND_JMP:
        case IR_STMT_STORE:
        case IR_STMT_INIT_FINAL:
            TODO;
    }
}

void llvm_emit_code(LlvmModule *module, IrCode *code) {
    for (size_t i = 0; i < vec_len(code->stmts); i++) {
        llvm_emit_stmt(module, code->stmts[i]);
    }
}
