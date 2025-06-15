#include "value.h"
#include "core/assert.h"
#include "core/null.h"
#include "ir/api/ir.h"
#include "sema/module/api/value.h"
#include "sema/module/ast/type.h"
#include "sema/module/decl.h"
#include "sema/module/exprs/expr.h"
#include "sema/module/module.h"
#include "sema/module/value.h"

static inline SemaRuntimeKind ast_value_kind_to_sema(AstValueDeclKind kind) {
    switch (kind) {
        case AST_VALUE_DECL_VAR: return SEMA_RUNTIME_VAR;
        case AST_VALUE_DECL_FINAL: return SEMA_RUNTIME_FINAL;
        case AST_VALUE_DECL_CONST: TODO;
    }
    UNREACHABLE;
}

static inline IrMutability ast_value_kind_to_ir(AstValueDeclKind kind) {
    switch (kind) {
        case AST_VALUE_DECL_VAR: return IR_MUTABLE;
        case AST_VALUE_DECL_FINAL: case AST_VALUE_DECL_CONST: return IR_IMMUTABLE;
    }
    UNREACHABLE;
}

static bool sema_module_emit_local_value(SemaModule *module, AstValueDecl *value_decl, SemaExprOutput *output) {
    AstValueInfo *info = value_decl->info;
    switch (info->kind) {
        case AST_VALUE_DECL_FINAL: {
            sema_ss_append_stmt(module->ss, ir_stmt_new_init_final(module->mempool, value_decl->sema.local_id,
                sema_expr_output_collect(output)));
            return true;
        }
        case AST_VALUE_DECL_VAR: {
            sema_ss_append_stmt(module->ss, ir_stmt_new_decl_var(module->mempool, value_decl->sema.local_id));
            sema_ss_append_stmt(module->ss, ir_stmt_new_store(module->mempool,
                ir_expr_new(vec_create_in(module->mempool, ir_expr_step_new_get_local(value_decl->sema.local_id))),
                sema_expr_output_collect(output)
            ));
            return true;
        }
        case AST_VALUE_DECL_CONST:
            TODO;
    }
    UNREACHABLE;
}

static SemaValueRuntime *sema_value_decl_get_initializer(SemaModule *module, SemaType *type, AstValueDecl *value_decl, SemaExprOutput *output) {
    SemaValueRuntime *value = NOT_NULL(sema_module_emit_runtime_expr(module, value_decl->initializer,
        sema_expr_ctx_new(output, type)));
    if (type && !sema_type_eq(type, value->type)) {
        sema_module_err(module, value_decl->initializer->slice, 
            "trying to initialize declaration with exlicit type $t with expression of type $t", type, value->type);
        return NULL;
    }
    if (!type) {
        type = value->type;
    }
    if (!type) {
        sema_module_err(module, value_decl->info->name,
            "unable to determine value type, specify type explicitly or set initializer");
        return NULL;
    }
    return value;
}

bool sema_module_stage_fill_value(SemaModule *module, AstValueDecl *value_decl) {
    AstValueInfo *info = value_decl->info;
    SemaType *type = info->explicit_type ? sema_module_type(module, info->explicit_type) : NULL;
    value_decl->sema.type = type;
    if (sema_module_is_global_scope(module)) {
        if (value_decl->info->kind == AST_VALUE_DECL_VAR) {
            IrDeclId id = ir_add_decl(module->ir);
            IrTypeId type_id = sema_type_ir_id(type);
            value_decl->sema.var_id = ir_init_var(module->ir, id, value_decl->global ? ir_var_new_global(type_id,
                value_decl->global->has_alias ? value_decl->global->alias : value_decl->info->name) : ir_var_new(type_id));
            sema_module_push_decl(module, value_decl->info->name, sema_decl_new(module->mempool,
                value_decl->info->is_public ? NULL : module, sema_value_new_runtime_global(module->mempool, 
                    SEMA_RUNTIME_VAR, type, id)));
            return true;
        } else if (value_decl->info->kind == AST_VALUE_DECL_CONST) {
            SemaValueRuntime *runtime = NOT_NULL(sema_value_decl_get_initializer(module, type, value_decl, NULL));
            SemaConst *constant = NOT_NULL(sema_value_runtime_should_be_constant(module,
                value_decl->initializer->slice, runtime));
            sema_module_push_decl(module, value_decl->info->name, sema_decl_new(module->mempool,
                value_decl->info->is_public ? NULL : module, sema_value_new_runtime_const(module->mempool,
                    sema_const_nest(module->mempool, constant, type))));
            return true;
        } else {
            sema_module_err(module, value_decl->info->name, "global value declaration can be a variable or constant only");
            return false;
        }
    } else {
        if (value_decl->global) {
            sema_module_err(module, value_decl->info->name, "local value declaration cannot be global value");
        }
        IrLocalId local_id = ir_func_add_local(module->ir, module->ss->func_id,
            ir_func_local_new(ast_value_kind_to_ir(info->kind), sema_type_ir_id(type)));
        value_decl->sema.local_id = local_id;
        sema_module_push_decl(module, info->name, sema_decl_new(module->mempool, info->is_public ? NULL : module,
            sema_value_new_runtime_local(module->mempool, ast_value_kind_to_sema(info->kind), type, local_id)));
        return true;
    }
}

bool sema_module_stage_emit_value(SemaModule *module, AstValueDecl *value_decl) {
    if (!value_decl->initializer) return true;
    if (sema_module_is_global_scope(module) && value_decl->info->kind == AST_VALUE_DECL_CONST) return true;
    SemaExprOutput output = sema_expr_output_new(module->mempool);
    SemaValueRuntime *runtime = NOT_NULL(sema_value_decl_get_initializer(module, value_decl->sema.type, value_decl,
        &output));
    if (sema_module_is_global_scope(module)) {
        assert(value_decl->info->kind == AST_VALUE_DECL_VAR);
        SemaConst *constant = NOT_NULL(sema_value_runtime_should_be_constant(module, value_decl->initializer->slice, runtime));
        ir_set_var_initializer(module->ir, value_decl->sema.var_id, sema_const_to_ir(module->mempool, constant));
        return true;
    } else {
        return sema_module_emit_local_value(module, value_decl, &output);
    }
}
