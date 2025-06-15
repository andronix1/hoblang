#include "fun.h"
#include "core/null.h"
#include "sema/module/decl.h"
#include "sema/module/stages/nodes/fun_info.h"
#include "sema/module/module.h"
#include "sema/module/stmts/body.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

bool sema_module_stage_fill_fun(SemaModule *module, AstFunDecl *fun_decl) {
    SemaType *type = fun_decl->sema.type = NOT_NULL(sema_func_info_type(module, fun_decl->info));
    IrMutability *args_mut = vec_new_in(module->mempool, IrMutability);
    vec_resize(args_mut, vec_len(type->function.args));
    for (size_t i = 0; i < vec_len(type->function.args); i++) {
        args_mut[i] = i == 0 && fun_decl->info->ext.is ? IR_IMMUTABLE : IR_MUTABLE;
    }
    IrDeclId decl_id = ir_add_decl(module->ir);
    sema_module_push_fun_info_decl(module, fun_decl->info, sema_decl_new(module->mempool,
        fun_decl->info->is_public ? NULL : module,
        sema_value_new_runtime_global(module->mempool, SEMA_RUNTIME_FINAL, type, decl_id)));
    IrTypeId type_id = sema_type_ir_id(type);
    fun_decl->sema.func_id = ir_init_func(module->ir, args_mut, decl_id, fun_decl->global ? 
        ir_func_new_global(fun_decl->global->has_alias ? fun_decl->global->alias : fun_decl->info->name, type_id) :
        ir_func_new(type_id));
    return true;
}

bool sema_module_stage_emit_fun(SemaModule *module, AstFunDecl *func) {
    SemaType *type = func->sema.type;
    assert(type->kind == SEMA_TYPE_FUNCTION);
    SemaScopeStack *old_ss = sema_module_swap_ss(module,
        sema_scope_stack_new(module->mempool, func->sema.func_id, type->function.returns));

    sema_module_push_scope(module);
    if (func->info->ext.is) {
        sema_module_push_decl(module, func->info->ext.self_name,
            sema_decl_new(module->mempool, module, sema_value_new_runtime_local(
                module->mempool,
                SEMA_RUNTIME_FINAL,
                type->function.args[0],
                ir_func_arg_local_id(module->ir, func->sema.func_id, 0)
            )
        ));
    }
    for (size_t i = 0; i < vec_len(func->info->args); i++) {
        sema_module_push_decl(module, func->info->args[i].name,
            sema_decl_new(module->mempool, module, sema_value_new_runtime_local(
                module->mempool,
                SEMA_RUNTIME_VAR,
                type->function.args[i + func->info->ext.is],
                ir_func_arg_local_id(module->ir, func->sema.func_id, i + func->info->ext.is)
            )
        ));
    }
    ir_init_func_body(module->ir, func->sema.func_id, sema_module_emit_code(module, func->body));
    sema_module_pop_scope(module);

    sema_module_swap_ss(module, old_ss);
    return true;
}
