#include "hir.h"
#include "core/assert.h"
#include "core/keymap.h"
#include "core/mempool.h"
#include "core/vec.h"
#include "hir/api/type.h"
#include "hir/stages/exprs.h"
#include "hir/hir.h"
#include <stdio.h>
#include <stdlib.h>

Hir *hir_new() {
    Hir *hir = malloc(sizeof(Hir));
    hir->mempool = mempool_new(128);
    hir->decls = vec_new_in(hir->mempool, HirDeclInfo);
    hir->funcs = vec_new_in(hir->mempool, HirFuncRecord);
    hir->vars = vec_new_in(hir->mempool, HirVarInfo);
    hir->externs_map = keymap_new_in(hir->mempool, HirExternInfo);
    hir->gen_scopes = vec_new_in(hir->mempool, HirGenScope);
    hir->root_gen_scopes = vec_new_in(hir->mempool, HirGenScopeId);
    hir->gen_params_count = 0;
    return hir;
}

void hir_free(Hir *hir) {
    mempool_free(hir->mempool);
    free(hir);
}

HirDeclId hir_add_decl(Hir *hir) {
    vec_push(hir->decls, hir_decl_info_new());
    return vec_len(hir->decls) - 1;
}

void hir_init_decl_func(Hir *hir, HirDeclId id, HirFuncId func) {
    hir_decl_info_fill_func(&hir->decls[id], func);
}

void hir_init_decl_extern(Hir *hir, HirDeclId id, HirExternId ext) {
    hir_decl_info_fill_extern(&hir->decls[id], ext);
}

void hir_init_decl_var(Hir *hir, HirDeclId id, HirVarId var) {
    hir_decl_info_fill_var(&hir->decls[id], var);
}

const HirDeclInfo *hir_get_decls(const Hir *hir) {
    return hir->decls;
}

HirExternId hir_add_extern(Hir *hir, Slice name, HirExternInfo info) {
    bool has_other = keymap_insert(hir->externs_map, name, info);
    assert(!has_other);
    return vec_len(hir->externs_map) - 1;
}

HirExternRecord hir_get_extern_info(const Hir *hir, HirExternId id) {
    keymap_at(hir->externs_map, id, info);
    return hir_extern_record_new(&info->value, info->key);
}

HirFuncId hir_register_fun(Hir *hir, HirType *type) {
    assert(type->kind == HIR_TYPE_FUNCTION);
    vec_push(hir->funcs, hir_func_record_new(type));
    return vec_len(hir->funcs) - 1;
}

inline HirFuncInfo *hir_get_mut_func_info(Hir *hir, HirFuncId id) {
    HirFuncRecord *rec = &hir->funcs[id];
    assert(rec->filled);
    return &rec->info;
}

void hir_init_fun(Hir *hir, HirFuncId id, HirMutability *args, HirFuncInfo info) {
    hir_func_info_fill(&hir->funcs[id], info);
    HirType *type = hir_get_mut_func_info(hir, id)->type;
    assert(type->kind == HIR_TYPE_FUNCTION);
    assert(vec_len(type->function.args) == vec_len(args));
    HirLocalId *args_locals = vec_new_in(hir->mempool, HirLocalId);
    vec_resize(args_locals, vec_len(args));
    for (size_t i = 0; i < vec_len(args); i++) {
        args_locals[i] = hir_fun_add_local(hir, id, hir_func_local_new(type->function.args[i], args[i]));
    }
    hir->funcs[id].args = args_locals;
}

void hir_init_fun_body(Hir *hir, HirFuncId id, HirCode *code) {
    HirFuncInfo *info = hir_get_mut_func_info(hir, id);
    assert(!info->code);
    info->code = code;
}

HirLocalId hir_fun_add_local(Hir *hir, HirFuncId id, HirFuncLocal local) {
    HirFuncInfo *info = hir_get_mut_func_info(hir, id);
    vec_push(info->locals, local);
    return vec_len(info->locals) - 1;
}

HirLoopId hir_fun_add_loop(Hir *hir, HirFuncId id) {
    HirFuncInfo *info = hir_get_mut_func_info(hir, id);
    return info->loops_count++;
}

inline const HirFuncInfo *hir_get_func_info(const Hir *hir, HirFuncId id) {
    HirFuncRecord *rec = &hir->funcs[id];
    assert(rec->filled);
    return &rec->info;
}

HirLocalId hir_get_func_arg_local(const Hir *hir, HirFuncId id, size_t number) {
    return hir->funcs[id].args[number];
}

static inline HirMutability hir_get_extern_mutability(HirExternKind kind) {
    switch (kind) {
        case HIR_EXTERN_FUNC: return HIR_IMMUTABLE;
        case HIR_EXTERN_VAR: return HIR_MUTABLE;
    }
    UNREACHABLE;
}

HirMutability hir_get_decl_mutability(Hir *hir, HirDeclId id) {
    HirDeclInfo *info = &hir->decls[id];
    assert(info->filled);
    switch (info->kind) {
        case HIR_DECL_FUNC: return HIR_IMMUTABLE;
        case HIR_DECL_EXTERN: {
            keymap_at(hir->externs_map, info->external, ext);
            return hir_get_extern_mutability(ext->value.kind);
        }
        case HIR_DECL_VAR: return HIR_MUTABLE;
    }
    UNREACHABLE;
}

void hir_postprocess(Hir *hir) {
    hir_fill_exprs(hir);
}

HirVarId hir_add_var(Hir *hir, HirVarInfo info) {
    vec_push(hir->vars, info);
    return vec_len(hir->vars) - 1;
}

void hir_set_var_initializer(Hir *hir, HirVarId id, HirConst initializer) {
    HirVarInfo *info = &hir->vars[id];
    info->initialized = true;
    info->initializer = initializer;
}

const HirVarInfo *hir_get_var_info(const Hir *hir, HirVarId id) {
    return &hir->vars[id];
}

size_t hir_get_vars_count(const Hir *hir) {
    return vec_len(hir->vars);
}

HirGenParamId hir_add_gen_param(Hir *hir) {
    return hir->gen_params_count++;
}

HirGenScopeId hir_add_gen_scope(Hir *hir) {
    vec_push(hir->gen_scopes, hir_gen_scope_new(hir->mempool));
    return vec_len(hir->gen_scopes) - 1;
}

void hir_gen_scope_add_param(Hir *hir, HirGenScopeId id, HirGenParamId param) {
    vec_push(hir->gen_scopes[id].params, param);
}

HirGenFuncId hir_gen_scope_add_func(Hir *hir, HirGenScopeId id, HirFuncId func) {
    vec_push(hir->gen_scopes[id].funcs, func);
    return vec_len(hir->gen_scopes[id].funcs) - 1;
}

void hir_gen_scope_add_scope(Hir *hir, HirGenScopeId id, HirGenScopeId scope) {
    vec_push(hir->gen_scopes[id].scopes, scope);
}

void hir_add_root_gen_scope(Hir *hir, HirGenScopeId id) {
    vec_push(hir->root_gen_scopes, id);
}

const HirGenScope *hir_get_gen_scope(const Hir *hir, HirGenScopeId id) {
    return &hir->gen_scopes[id];
}

const HirGenScopeId *hir_get_root_gen_scopes(const Hir *hir) {
    return hir->root_gen_scopes;
}

size_t hir_get_gen_params_count(const Hir *hir) {
    return hir->gen_params_count;
}
