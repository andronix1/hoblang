#include "funcs.h"
#include "llvm/module/emit/expr.h"
#include "llvm/module/emit/func.h"
#include "llvm/module/module.h"
#include <llvm-c/Core.h>
#include "ir/ir.h"

void llvm_module_setup_funcs(LlvmModule *module) {
    for (size_t i = 0; i < vec_len(module->ir->funcs); i++) {
        llvm_emit_func_body(module, i);
    }
    for (size_t i = 0; i < vec_len(module->ir->vars); i++) {
        IrVarInfo *info = &module->ir->vars[i];
        LLVMSetInitializer(module->decls[info->decl_id], info->var.initializer ? 
            llvm_emit_const(module, info->var.initializer) : LLVMGetUndef(module->types[info->var.type]));
        llvm_emit_func_body(module, i);
    }
}
