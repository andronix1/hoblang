#include "function.h"
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

SemaValue *sema_module_emit_expr_function(SemaModule *module, AstExprFunc *func) {
    SemaType **args = vec_new_in(module->mempool, SemaType*);
    vec_resize(args, vec_len(func->args));
    for (size_t i = 0; i < vec_len(func->args); i++) {
        args[i] = NOT_NULL(sema_module_type(module, func->args[i].type));
    }
    SemaType *type = sema_type_new_function(module, args, NOT_NULL(sema_module_opt_type(module, func->returns)));

    HirMutability *args_mut = vec_new_in(module->mempool, HirMutability);
    vec_resize(args_mut, vec_len(type->function.args));
    for (size_t i = 0; i < vec_len(type->function.args); i++) {
        args_mut[i] = HIR_MUTABLE;
    }

    HirDeclId decl_id = hir_add_decl(module->hir);
    HirFuncId func_id = hir_register_fun(module->hir, sema_type_hir_id(type));
    hir_init_decl_func(module->hir, decl_id, func_id);

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
    sema_module_pop_scope(module);
    sema_module_swap_ss(module, old_ss);

    return sema_value_new_runtime_const(module->mempool, sema_const_new_func(module->mempool, type, decl_id));
}

