#include "code.h"
#include "core/assert.h"
#include "ir/stmt/code.h"
#include "ir/stmt/stmt.h"
#include "ir/ir.h"
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
        case IR_STMT_STORE:
            LLVMBuildStore(module->builder,
                llvm_emit_expr(module, &stmt->store.rvalue, true),
                llvm_emit_expr(module, &stmt->store.lvalue, false)
            );
            break;
        case IR_STMT_DECL_VAR:
            module->func.locals[stmt->var_id] = llvm_alloca(module, module->types[
                module->ir->funcs[module->func.id].locals[stmt->var_id].type]);
            break;
        case IR_STMT_INIT_FINAL:
            module->func.locals[stmt->var_id] = llvm_emit_expr(module,
                &stmt->init_final.value, true);
            break;
        case IR_STMT_COND_JMP:
            TODO;
    }
}

void llvm_emit_code(LlvmModule *module, IrCode *code) {
    for (size_t i = 0; i < vec_len(code->stmts); i++) {
        llvm_emit_stmt(module, code->stmts[i]);
    }
}
