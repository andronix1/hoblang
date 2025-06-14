#include "code.h"
#include "ir/stmt/code.h"
#include "ir/stmt/stmt.h"
#include "ir/ir.h"
#include "llvm/module/emit/expr.h"
#include "llvm/module/module.h"
#include <llvm-c/Core.h>
#include <llvm-c/Types.h>
#include <stdio.h>

static void llvm_emit_cond_jmp(LlvmModule *module, IrStmtCondJmp *cond_jmp) {
    /*
        .c1:
            cjmp a .b1, .c2
        .b1:
            ...
            jmp .e
        .c2:
            cjmp a .b2, .ce
        .b2:
            ...
            jmp .e
        .ce:
            ...
            jmp .e
        .e:
            ...
    */
    LLVMBasicBlockRef end = LLVMAppendBasicBlock(module->func.value, "");
    LLVMBasicBlockRef final_end = cond_jmp->flow == IR_CODE_FLOW_PASSED ?
        LLVMAppendBasicBlock(module->func.value, "") : NULL;
    for (size_t i = 0; i < vec_len(cond_jmp->conds); i++) {
        IrStmtCondJmpBlock *block = &cond_jmp->conds[i];
        LLVMBasicBlockRef body = LLVMAppendBasicBlock(module->func.value, "");
        LLVMValueRef value = llvm_emit_expr(module, &block->cond, true);
        LLVMBuildCondBr(module->builder, value, body, end);
        LLVMPositionBuilderAtEnd(module->builder, body);
        llvm_emit_code(module, block->code);
        if (block->code->flow == IR_CODE_FLOW_PASSED) {
            LLVMBuildBr(module->builder, final_end);
        }
        LLVMPositionBuilderAtEnd(module->builder, end);
    }
    if (cond_jmp->else_code) {
        llvm_emit_code(module, cond_jmp->else_code);
        if (cond_jmp->else_code->flow == IR_CODE_FLOW_PASSED) {
            LLVMBuildBr(module->builder, final_end);
        }
    } else {
        LLVMBuildBr(module->builder, final_end);
    }
    LLVMPositionBuilderAtEnd(module->builder, final_end);
}

static void llvm_emit_loop(LlvmModule *module, IrStmtLoop *loop) {
    LLVMBasicBlockRef begin = LLVMAppendBasicBlock(module->func.value, "");
    LLVMBasicBlockRef end = LLVMAppendBasicBlock(module->func.value, "");
    module->func.loops[loop->id] = llvm_loop_info_new(begin, end);

    LLVMBuildBr(module->builder, begin);

    LLVMPositionBuilderAtEnd(module->builder, begin);
    llvm_emit_code(module, loop->code);
    if (loop->code->flow == IR_CODE_FLOW_PASSED) {
        LLVMBuildBr(module->builder, begin);
    }

    LLVMPositionBuilderAtEnd(module->builder, end);
}

static void llvm_emit_stmt(LlvmModule *module, IrStmt *stmt) {
    switch (stmt->kind) {
        case IR_STMT_EXPR:
            llvm_emit_expr(module, &stmt->expr, true);
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
            module->func.locals[stmt->var_id] = llvm_emit_expr(module, &stmt->init_final.value, true);
            break;
        case IR_STMT_COND_JMP:
            llvm_emit_cond_jmp(module, &stmt->cond_jmp);
            break;
        case IR_STMT_LOOP:
            llvm_emit_loop(module, &stmt->loop);
            break;
        case IR_STMT_BREAK:
            LLVMBuildBr(module->builder, module->func.loops[stmt->break_loop.id].end);
            break;
        case IR_STMT_CONTINUE:
            LLVMBuildBr(module->builder, module->func.loops[stmt->break_loop.id].begin);
            break;
    }
}

void llvm_emit_code(LlvmModule *module, IrCode *code) {
    for (size_t i = 0; i < vec_len(code->stmts); i++) {
        llvm_emit_stmt(module, code->stmts[i]);
    }
}
