#include "llvm/module/module.h"
#include "llvm/module/nodes/expr.h"
#include "stmt.h"
#include "ast/stmt.h"
#include <llvm-c/Core.h>

void llvm_emit_stmt(LlvmModule *module, AstStmt *stmt) {
    switch (stmt->kind) {
        case AST_STMT_EXPR:
            llvm_emit_expr(module, stmt->expr);
            break;
        case AST_STMT_RETURN:
            stmt->ret.value ?
                LLVMBuildRet(module->builder, llvm_emit_expr_get(module, stmt->ret.value)) :
                LLVMBuildRetVoid(module->builder);
            break;
    }
}
