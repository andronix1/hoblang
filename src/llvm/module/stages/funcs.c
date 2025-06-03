#include "funcs.h"
#include "llvm/module/emit/func.h"
#include "llvm/module/module.h"
#include "ir/ir.h"

void llvm_module_setup_funcs(LlvmModule *module) {
    for (size_t i = 0; i < vec_len(module->ir->funcs); i++) {
        llvm_emit_func_body(module, i);
    }
}
