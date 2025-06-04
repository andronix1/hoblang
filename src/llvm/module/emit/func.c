#include "func.h"
#include "ir/api/ir.h"
#include "ir/ir.h"
#include "llvm/module/emit/code.h"
#include "llvm/module/module.h"
#include <llvm-c/Core.h>

void llvm_emit_func_body(LlvmModule *module, IrFuncId id) {
    LLVMValueRef func = module->decls[ir_func_decl_id(module->ir, id)];
    llvm_func_ctx_set(module, id);
    IrFuncInfo *info = &module->ir->funcs[id];
    for (size_t i = 0; i < vec_len(info->args); i++) {
        LLVMValueRef value = LLVMGetParam(func, i);
        IrFuncArg *arg = &info->func.args[i];
        if (arg->mutability == IR_MUTABLE) {
            LLVMValueRef new_value = llvm_alloca(module, module->types[arg->type]);
            LLVMBuildStore(module->builder, value, new_value);
            value = new_value;
        }
        module->func.locals[info->args[i]] = value;
    }
    LLVMPositionBuilderAtEnd(module->builder, LLVMAppendBasicBlock(func, ""));
    llvm_emit_code(module, module->ir->funcs[id].func.code);
}
