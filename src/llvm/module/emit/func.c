#include "func.h"
#include "ir/api/ir.h"
#include "ir/ir.h"
#include "llvm/module/emit/code.h"
#include "llvm/module/module.h"
#include <llvm-c/Core.h>

void llvm_emit_func_body(LlvmModule *module, IrFuncId id) {
    LLVMValueRef func = module->decls[ir_func_decl_id(module->ir, id)];
    llvm_func_ctx_set(module, id, func);
    IrFuncInfo *info = &module->ir->funcs[id];
    IrTypeId func_type_id = ir_type_record_resolve_simple(module->ir, info->func.type_id);
    IrType *func_type = &module->ir->types[func_type_id].simple;
    for (size_t i = 0; i < vec_len(info->args); i++) {
        LLVMValueRef value = LLVMGetParam(func, i);
        if (info->locals[info->args[i]].mutability == IR_MUTABLE) {
            LLVMValueRef new_value = llvm_alloca(module,
                module->types[func_type->function.args[i]]);
            LLVMBuildStore(module->builder, value, new_value);
            value = new_value;
        }
        module->func.locals[info->args[i]] = value;
    }
    LLVMPositionBuilderAtEnd(module->builder, LLVMAppendBasicBlock(func, ""));
    llvm_emit_code(module, module->ir->funcs[id].func.code);
}
