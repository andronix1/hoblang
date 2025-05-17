#include "llvm/module/module.h"
#include "llvm/module/nodes/expr.h"
#include "llvm/module/nodes/type.h"
#include "stmt.h"
#include "ast/stmt.h"
#include "sema/module/api/value.h"
#include <llvm-c/Core.h>

void llvm_emit_stmt(LlvmModule *module, AstStmt *stmt) {
    switch (stmt->kind) {
        case AST_STMT_EXPR:
            llvm_emit_expr(module, stmt->expr);
            break;
        case AST_STMT_ASSIGN: {
            LLVMValueRef dst = llvm_emit_expr(module, stmt->assign.dst);
            LLVMValueRef what = llvm_emit_expr_get(module, stmt->assign.what);
            if (stmt->assign.short_assign.is) {
                what = llvm_emit_binop(module,
                    LLVMBuildLoad2(module->builder, llvm_type(module,
                        sema_value_is_runtime(stmt->assign.dst->sema.value)), dst, ""),
                    what, stmt->assign.short_assign.kind);
            }
            LLVMBuildStore(module->builder, what, dst);
            break;
        }
        case AST_STMT_RETURN:
            stmt->ret.value ?
                LLVMBuildRet(module->builder, llvm_emit_expr_get(module, stmt->ret.value)) :
                LLVMBuildRetVoid(module->builder);
            break;
    }
}
