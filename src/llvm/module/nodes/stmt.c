#include "llvm/module/module.h"
#include "llvm/module/nodes/body.h"
#include "llvm/module/nodes/expr.h"
#include "llvm/module/nodes/type.h"
#include "stmt.h"
#include "ast/stmt.h"
#include "ast/body.h"
#include "sema/module/api/value.h"
#include <llvm-c/Core.h>
#include <llvm-c/Types.h>

static void llvm_cond_block(LlvmModule *module, AstCondBlock *block, LLVMBasicBlockRef end_block) {
    LLVMBasicBlockRef body_block = LLVMAppendBasicBlock(module->state.func, "");
    LLVMBasicBlockRef next_block = LLVMAppendBasicBlock(module->state.func, "");
    LLVMBuildCondBr(module->builder, llvm_emit_expr_get(module, block->cond),
        body_block, next_block);
    LLVMPositionBuilderAtEnd(module->builder, body_block);
    llvm_emit_body(module, block->body);
    if (!block->body->sema.finished) {
        LLVMBuildBr(module->builder, end_block);
    }
    LLVMPositionBuilderAtEnd(module->builder, next_block);
}

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
        case AST_STMT_IF: {
            LLVMBasicBlockRef end_block = stmt->if_else.sema.breaks ?
                NULL : LLVMAppendBasicBlock(module->state.func, "");
            for (size_t i = 0; i < vec_len(stmt->if_else.conds); i++) {
                llvm_cond_block(module, &stmt->if_else.conds[i], end_block);
            }
            if (stmt->if_else.else_body) {
                llvm_emit_body(module, stmt->if_else.else_body);
                if (!stmt->if_else.else_body->sema.finished) {
                    LLVMBuildBr(module->builder, end_block);
                }
            }
            if (end_block) {
                LLVMPositionBuilderAtEnd(module->builder, end_block);
            }
            break;
        }
    }
}
