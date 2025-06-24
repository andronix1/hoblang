#include "function.h"
#include "ast/body.h"
#include "core/mempool.h"
#include "core/null.h"
#include "core/vec.h"
#include "hir/api/hir.h"
#include "sema/module/ast/type.h"
#include "sema/module/const.h"
#include "sema/module/decl.h"
#include "sema/module/module.h"
#include "sema/module/stmts/body.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

SemaValue *sema_module_emit_expr_function(SemaModule *module, AstExprFunc *func, Slice where, SemaExprCtx ctx) {
    SemaType **args = vec_new_in(module->mempool, SemaType*);
    vec_resize(args, vec_len(func->args));
    SemaType *expecting = ctx.expectation;
    if (expecting) {
        expecting = sema_type_root(expecting);
    }
    if (expecting && (expecting->kind != SEMA_TYPE_FUNCTION ||
            vec_len(expecting->function.args) != vec_len(func->args))) {
        expecting = NULL;
    }
    for (size_t i = 0; i < vec_len(func->args); i++) {
        AstType *type = func->args[i].type;
        if (!type) {
            if (!expecting) {
                sema_module_err(module, where, "type infering specified but there is no type expected");
                return NULL;
            }
            args[i] = expecting->function.args[i];
        } else {
            args[i] = NOT_NULL(sema_module_type(module, type));
        }
    }
    SemaType *returns = NULL;
    if (expecting && !func->returns) {
        returns = expecting->function.returns;
    } else {
        returns = NOT_NULL(sema_module_opt_type(module, func->returns));
    }
    SemaType *type = sema_type_new_function(module, args, returns);

    HirMutability *args_mut = vec_new_in(module->mempool, HirMutability);
    vec_resize(args_mut, vec_len(type->function.args));
    for (size_t i = 0; i < vec_len(type->function.args); i++) {
        args_mut[i] = HIR_MUTABLE;
    }

    SemaValue *value = NULL;
    HirFuncId func_id = hir_register_fun(module->hir, sema_type_hir_id(type));
    if (vec_len(module->gen_scopes)) {
        HirGenScopeId scope = *vec_top(module->gen_scopes);
        HirGenFuncId gen_func_id = hir_gen_scope_add_func(module->hir, scope, func_id);
        value = sema_value_new_runtime_const(module->mempool, sema_const_new_gen_func(module->mempool, type, scope,
            gen_func_id, hir_get_gen_scope(module->hir, scope)->params));
    } else {
        HirDeclId decl_id = hir_add_decl(module->hir);
        hir_init_decl_func(module->hir, decl_id, func_id);
        value = sema_value_new_runtime_const(module->mempool, sema_const_new_func(module->mempool, type, decl_id));
    }

    SemaScopeStack *old_ss = sema_module_swap_ss(module, sema_scope_stack_new(module->mempool, func_id,
        type->function.returns));

    sema_module_push_scope(module, NULL);
    hir_init_fun(module->hir, func_id, args_mut, hir_func_info_new(module->mempool, opt_slice_new_null()));
    for (size_t i = 0; i < vec_len(func->args); i++) {
        sema_module_push_decl(module, func->args[i].name,
            sema_decl_new(module->mempool, module, sema_value_new_runtime_local(
                module->mempool,
                SEMA_RUNTIME_VAR,
                type->function.args[i],
                hir_get_func_arg_local(module->hir, func_id, i)
            )
        ));
    }
    hir_init_fun_body(module->hir, func_id, sema_module_emit_code(module, func->body, NULL));
    if (!func->body->sema.breaks && !sema_type_eq(type->function.returns, sema_type_new_void(module))) {
        sema_module_err(module, where, "expected function to return value but its body passes");
    }
    sema_module_pop_scope(module);
    sema_module_swap_ss(module, old_ss);

    return value;
}

