#include "decls.h"
#include "ir/ir.h"
#include "llvm/module/module.h"
#include <llvm-c/Core.h>

static void llvm_module_setup_externs(LlvmModule *module) {
    for (size_t i = 0; i < vec_len(module->ir->externs); i++) {
        IrExternInfo *info = &module->ir->externs[i];
        LLVMValueRef value = NULL;
        switch (info->ext.kind) {
            case IR_EXTERN_FUNC:
                value = LLVMAddFunction(module->module,
                    mempool_slice_to_cstr(module->mempool, info->ext.name),
                    module->types[info->ext.type]
                );
                break;
            case IR_EXTERN_VAR:
                value = LLVMAddGlobal(module->module,
                    module->types[info->ext.type],
                    mempool_slice_to_cstr(module->mempool, info->ext.name)
                );
                break;
        }
        assert(value);
        module->decls[info->decl_id] = value;
    }
}

static void llvm_module_setup_funcs(LlvmModule *module) {
    for (size_t i = 0; i < vec_len(module->ir->funcs); i++) {
        IrFuncInfo *info = &module->ir->funcs[i];
        module->decls[info->decl_id] = LLVMAddFunction(module->module, 
            info->func.is_global ? 
                mempool_slice_to_cstr(module->mempool, info->func.global_name) :
                "",
            module->types[info->type_id]);
    }
}

void llvm_module_setup_decls(LlvmModule *module) {
    module->decls = vec_new_in(module->mempool, LLVMValueRef);
    vec_resize(module->decls, vec_len(module->ir->decls));
    for (size_t i = 0; i < vec_len(module->ir->decls); i++) {
        module->decls[i] = NULL;
    }
    llvm_module_setup_funcs(module);
    llvm_module_setup_externs(module);
    for (size_t i = 0; i < vec_len(module->ir->decls); i++) {
        assert(module->decls[i]);
    }
}
