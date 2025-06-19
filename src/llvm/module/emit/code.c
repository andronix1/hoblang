#include "code.h"
#include "hir/api/code.h"
#include "llvm/module/emit/expr.h"
#include "llvm/module/module.h"
#include "llvm/module/types.h"
#include <llvm-c/Core.h>
#include <llvm-c/Types.h>
#include <stdio.h>

static void llvm_emit_cond_jmp(LlvmModule *module, HirStmtCondJmp *cond_jmp) {
    LLVMBasicBlockRef end = LLVMAppendBasicBlock(module->func.value, "");
    LLVMBasicBlockRef final_end = cond_jmp->breaks ? NULL : LLVMAppendBasicBlock(module->func.value, "");
    for (size_t i = 0; i < vec_len(cond_jmp->conds); i++) {
        HirStmtCondJmpBlock *block = &cond_jmp->conds[i];
        LLVMBasicBlockRef body = LLVMAppendBasicBlock(module->func.value, "");
        LLVMValueRef value = llvm_emit_expr(module, &block->cond, true);
        LLVMBuildCondBr(module->builder, value, body, end);
        llvm_module_set_block(module, body);
        llvm_emit_code(module, block->code);
        if (!block->code->breaks) {
            LLVMBuildBr(module->builder, final_end);
        }
        llvm_module_set_block(module, end);
    }
    if (cond_jmp->else_code) {
        llvm_emit_code(module, cond_jmp->else_code);
        if (!cond_jmp->else_code->breaks) {
            LLVMBuildBr(module->builder, final_end);
        }
    } else {
        LLVMBuildBr(module->builder, final_end);
    }
    llvm_module_set_block(module, final_end);
}

static void llvm_emit_loop(LlvmModule *module, HirStmtLoop *loop) {
    LLVMBasicBlockRef begin = LLVMAppendBasicBlock(module->func.value, "");
    LLVMBasicBlockRef end = LLVMAppendBasicBlock(module->func.value, "");
    module->func.loops[loop->id] = llvm_loop_info_new(begin, end);

    LLVMBuildBr(module->builder, begin);

    llvm_module_set_block(module, begin);
    llvm_emit_code(module, loop->code);
    if (!loop->code->breaks) {
        LLVMBuildBr(module->builder, begin);
    }

    llvm_module_set_block(module, end);
}

static void llvm_emit_stmt(LlvmModule *module, HirStmt *stmt) {
    switch (stmt->kind) {
        case HIR_STMT_EXPR:
            llvm_emit_expr(module, &stmt->expr, true);
            break;
        case HIR_STMT_RET:
            LLVMBuildRet(module->builder, llvm_emit_expr(module, &stmt->ret.value, true));
            break;
        case HIR_STMT_RET_VOID:
            LLVMBuildRetVoid(module->builder);
            break;
        case HIR_STMT_STORE:
            LLVMBuildStore(module->builder,
                llvm_emit_expr(module, &stmt->store.rvalue, true),
                llvm_emit_expr(module, &stmt->store.lvalue, false)
            );
            break;
        case HIR_STMT_DECL_VAR: {
            const HirFuncInfo *info = hir_get_func_info(module->hir, module->func.id);
            module->func.locals[stmt->var_id] = llvm_alloca(module, llvm_runtime_type(module,
                info->locals[stmt->var_id].type));
            break;
        }
        case HIR_STMT_INIT_FINAL:
            module->func.locals[stmt->var_id] = llvm_emit_expr(module, &stmt->init_final.value, true);
            break;
        case HIR_STMT_COND_JMP:
            llvm_emit_cond_jmp(module, &stmt->cond_jmp);
            break;
        case HIR_STMT_LOOP:
            llvm_emit_loop(module, &stmt->loop);
            break;
        case HIR_STMT_BREAK:
            LLVMBuildBr(module->builder, module->func.loops[stmt->break_loop.id].end);
            break;
        case HIR_STMT_CONTINUE:
            LLVMBuildBr(module->builder, module->func.loops[stmt->break_loop.id].begin);
            break;
    }
}

void llvm_emit_code(LlvmModule *module, const HirCode *code) {
    for (size_t i = 0; i < vec_len(code->stmts); i++) {
        llvm_emit_stmt(module, &code->stmts[i]);
    }
}
