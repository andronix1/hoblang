#include "func.h"
#include "ir/api/ir.h"
#include "ir/ir.h"
#include "llvm/module/emit/code.h"
#include "llvm/module/module.h"
#include <llvm-c/Core.h>

void llvm_emit_func_body(LlvmModule *module, IrFuncId id) {
    LLVMValueRef func = module->decls[ir_func_decl_id(module->ir, id)];
    LLVMPositionBuilderAtEnd(module->builder, LLVMAppendBasicBlock(func, ""));
    llvm_emit_code(module, module->ir->funcs[id].func.code);
}
