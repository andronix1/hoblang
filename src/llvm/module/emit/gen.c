#include "gen.h"
#include "llvm/module/emit/func.h"
#include "llvm/module/module.h"
#include "llvm/module/types.h"

static inline void llvm_setup_gen_params(LlvmModule *module, HirGenParamId *params, HirTypeId *input) {
    for (size_t i = 0; i < vec_len(params); i++) {
        HirGenParamId param_id = params[i];
        assert(!module->gen_params[param_id]);
        module->gen_params[param_id] = llvm_runtime_type(module, input[i]);
    }
}

static inline void llvm_clean_gen_params(LlvmModule *module, HirGenParamId *params) {
    for (size_t i = 0; i < vec_len(params); i++) {
        module->gen_params[params[i]] = NULL;
    }
}

static void llvm_setup_gen_usage(LlvmModule *module, HirGenScopeId id, HirGenUsageId usage) {
    const HirGenScope *scope = hir_get_gen_scope(module->hir, id);
    llvm_setup_gen_params(module, scope->params, scope->usages[usage]);
    for (size_t i = 0; i < vec_len(scope->funcs); i++) {
        module->gen_scopes[id].funcs[i][usage] = llvm_setup_func(module, scope->funcs[i]);
    }
    llvm_clean_gen_params(module, scope->params);
}

static void llvm_emit_usage(LlvmModule *module, HirGenScopeId id, HirGenUsageId usage) {
    const HirGenScope *scope = hir_get_gen_scope(module->hir, id);
    llvm_setup_gen_params(module, scope->params, scope->usages[usage]);
    for (size_t i = 0; i < vec_len(scope->funcs); i++) {
        llvm_emit_func(module, scope->funcs[i], module->gen_scopes[id].funcs[i][usage]);
    }
    llvm_clean_gen_params(module, scope->params);
}

void llvm_setup_gen(LlvmModule *module, HirGenScopeId id) {
    const HirGenScope *scope = hir_get_gen_scope(module->hir, id);
    for (size_t i = 0; i < vec_len(scope->usages); i++) {
        llvm_setup_gen_usage(module, id, i);
    }
}

void llvm_emit_gen(LlvmModule *module, HirGenScopeId id) {
    const HirGenScope *scope = hir_get_gen_scope(module->hir, id);
    for (size_t i = 0; i < vec_len(scope->scopes); i++) {
        llvm_setup_gen(module, scope->scopes[i]);
        llvm_emit_gen(module, scope->scopes[i]);
    }
    for (size_t i = 0; i < vec_len(scope->usages); i++) {
        llvm_emit_usage(module, id, i);
    }
}
