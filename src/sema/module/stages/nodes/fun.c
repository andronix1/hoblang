#include "fun.h"
#include "core/null.h"
#include "sema/module/ast/generic.h"
#include "sema/module/ast/global.h"
#include "sema/module/const.h"
#include "sema/module/decl.h"
#include "sema/module/stages/nodes/fun_info.h"
#include "sema/module/module.h"
#include "sema/module/stmts/body.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

static inline void sema_module_push_func_ctx(SemaModule *module, AstFunDecl *func, SemaGenericCtx *ctx) {
    if (func->info->generic) {
        *ctx = sema_module_generic_ctx_setup(module, func->info->generic, func->sema.generic);
    }

    if (func->info->ext.sema.func_generic) {
        *ctx = sema_module_generic_ctx_setup(module, func->info->ext.sema.generic->type.source,
            func->info->ext.sema.func_generic);
    }
}

static inline SemaGeneric *sema_module_func_get_gen(AstFunDecl *func) {
    SemaGeneric *generic = func->sema.generic;
    if (!generic) {
        generic = func->info->ext.sema.func_generic;
    }
    return generic;
}

bool sema_module_stage_fill_fun(SemaModule *module, AstFunDecl *func) {
    func->sema.generic = NULL;
    SemaGenericCtx ctx;

    NOT_NULL(sema_func_info_setup(module, func->info));

    if (func->info->generic) {
        SemaGeneric *generic = NOT_NULL(sema_module_generic_func(module, func->info->generic, func->info->name));
        func->sema.generic = generic;
    }

    sema_module_push_func_ctx(module, func, &ctx);

    SemaType *type = NOT_NULL(sema_func_info_type(module, func->info));
    func->sema.type = type;

    HirFuncId func_id = hir_register_fun(module->hir, sema_type_to_hir(module, type));
    func->sema.func_id = func_id;

    SemaGeneric *generic = sema_module_func_get_gen(func);
    if (generic) {
        HirGenFuncId id = hir_gen_scope_add_func(module->hir, generic->func.scope, func_id);
        sema_generic_fill_func(generic, type, id);
        sema_module_generic_ctx_clean(module, ctx);
    }

    HirMutability *args_mut = vec_new_in(module->mempool, HirMutability);
    vec_resize(args_mut, vec_len(type->function.args));
    for (size_t i = 0; i < vec_len(type->function.args); i++) {
        args_mut[i] = i == 0 && func->info->ext.is ? HIR_IMMUTABLE : HIR_MUTABLE;
    }

    if (!generic) {
        HirDeclId decl_id = hir_add_decl(module->hir);
        sema_module_push_fun_info_decl(module, func->info, sema_value_new_runtime_const(module->mempool,
            sema_const_new_func(module->mempool, type, decl_id)));
        hir_init_decl_func(module->hir, decl_id, func_id);
    } else {
        sema_module_push_fun_info_decl(module, func->info, sema_value_new_generic(module->mempool, generic));
    }

    hir_init_fun(module->hir, func_id, args_mut, hir_func_info_new(module->mempool,
        sema_global_to_opt_slice(func->global, func->info->name)));
    return true;
}

bool sema_module_stage_emit_fun(SemaModule *module, AstFunDecl *func) {
    SemaType *type = func->sema.type;
    assert(type->kind == SEMA_TYPE_FUNCTION);

    HirFuncId func_id = func->sema.func_id;

    SemaScopeStack *old_ss = sema_module_swap_ss(module, sema_scope_stack_new(module->mempool, func_id,
        type->function.returns));

    SemaGenericCtx ctx;
    sema_module_push_func_ctx(module, func, &ctx);

    sema_module_push_scope(module, NULL);
    if (func->info->ext.is) {
        sema_module_push_decl(module, func->info->ext.self_name, sema_decl_new(module->mempool, module,
            sema_value_new_runtime_local(module->mempool, SEMA_RUNTIME_FINAL, type->function.args[0],
                hir_get_func_arg_local(module->hir, func_id, 0))));
    }
    for (size_t i = 0; i < vec_len(func->info->args); i++) {
        size_t arg_id = i + func->info->ext.is;
        sema_module_push_decl(module, func->info->args[i].name,
            sema_decl_new(module->mempool, module, sema_value_new_runtime_local(module->mempool, SEMA_RUNTIME_VAR,
                type->function.args[arg_id], hir_get_func_arg_local(module->hir, func_id, arg_id))));
    }
    hir_init_fun_body(module->hir, func_id, sema_module_emit_code(module, func->body, NULL));
    if (!func->body->sema.breaks && !sema_type_eq(type->function.returns, sema_type_new_void(module->mempool))) {
        sema_module_err(module, func->info->name, "expected function to return value but its body passes");
    }

    sema_module_pop_scope(module);

    if (sema_module_func_get_gen(func)) {
        sema_module_generic_ctx_clean(module, ctx);
    }

    sema_module_swap_ss(module, old_ss);
    return true;
}
