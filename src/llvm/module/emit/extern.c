#include "extern.h"
#include "core/assert.h"
#include "hir/api/hir.h"
#include "llvm/module/module.h"
#include "llvm/module/types.h"
#include <llvm-c/Core.h>

void llvm_setup_extern(LlvmModule *module, HirExternId id, HirDeclId decl) {
    HirExternRecord rec = hir_get_extern_info(module->hir, id);
    switch (rec.info->kind) {
        case HIR_EXTERN_FUNC:
            module->decls[decl] = LLVMAddFunction(module->module, mempool_slice_to_cstr(module->mempool, rec.name),
                llvm_function_type(module, rec.info->type));
            return;
        case HIR_EXTERN_VAR:
            module->decls[decl] = LLVMAddGlobal(module->module, llvm_runtime_type(module, rec.info->type),
                mempool_slice_to_cstr(module->mempool, rec.name));
            return;
    }
    UNREACHABLE;
}
