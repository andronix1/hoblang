#include "value.h"
#include "core/assert.h"
#include "core/null.h"
#include "core/opt_slice.h"
#include "hir/api/hir.h"
#include "hir/api/var.h"
#include "sema/module/api/const.h"
#include "sema/module/api/type.h"
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

static inline HirMutability ast_value_kind_to_hir(AstValueDeclKind kind) {
    switch (kind) {
        case AST_VALUE_DECL_VAR: return HIR_MUTABLE;
        case AST_VALUE_DECL_FINAL: case AST_VALUE_DECL_CONST: return HIR_IMMUTABLE;
    }
    UNREACHABLE;
}

static SemaValueRuntime *sema_value_decl_get_initializer(SemaModule *module, AstValueDecl *value_decl, SemaExprOutput *output) {
    SemaType *type = value_decl->sema.type;
    SemaValueRuntime *value = NOT_NULL(sema_module_emit_runtime_expr_full(module, value_decl->initializer,
        sema_expr_ctx_new(output, type)));
    if (type && !sema_type_eq(type, value->type)) {
        sema_module_err(module, value_decl->initializer->slice, 
            "trying to initialize declaration with exlicit type $t with expression of type $t", type, value->type);
        return NULL;
    }
    if (!type) {
        value_decl->sema.type = type = value->type;
    }
    return value;
}

bool sema_module_stage_fill_global(SemaModule *module, AstValueDecl *value_decl) {
    AstValueInfo *info = value_decl->info;
    if (!value_decl->sema.type) {
        sema_module_err(module, value_decl->info->name, "global declaration type must be specified explicitly");
        return false;
    }
    switch (info->kind) {
        case AST_VALUE_DECL_VAR: {
            HirDeclId decl_id = hir_add_decl(module->hir);
            HirVarId var_id = value_decl->sema.var_id = hir_add_var(module->hir, hir_var_info_new(value_decl->global ?
                opt_slice_new_value(value_decl->global->has_alias ? value_decl->global->alias : value_decl->info->name) :
                opt_slice_new_null(), sema_type_hir_id(value_decl->sema.type)));
            hir_init_decl_var(module->hir, decl_id, var_id);
            sema_module_push_decl(module, value_decl->info->name, sema_decl_new(module->mempool,
                value_decl->info->is_public ? NULL : module, sema_value_new_runtime_global(module->mempool, 
                    SEMA_RUNTIME_VAR, value_decl->sema.type, decl_id)));
            return true;
        }
        case AST_VALUE_DECL_CONST: {
            SemaValueRuntime *runtime = NOT_NULL(sema_value_decl_get_initializer(module, value_decl, NULL));
            SemaConst *constant = NOT_NULL(sema_value_runtime_should_be_constant(module,
                value_decl->initializer->slice, runtime));
            sema_module_push_decl(module, value_decl->info->name, sema_decl_new(module->mempool,
                value_decl->info->is_public ? NULL : module, sema_value_new_runtime_const(module->mempool,
                    sema_const_nest(module->mempool, constant, value_decl->sema.type))));
            return true;
        }
        case AST_VALUE_DECL_FINAL:
            sema_module_err(module, value_decl->info->name, "global value cannot be final");
            return false;
    }
    UNREACHABLE;
}
bool sema_module_stage_emit_global(SemaModule *module, AstValueDecl *value_decl) {
    AstValueInfo *info = value_decl->info;
    switch (info->kind) {
        case AST_VALUE_DECL_VAR: {
            SemaValueRuntime *runtime = NOT_NULL(sema_value_decl_get_initializer(module, value_decl, NULL));
            SemaConst *constant = NOT_NULL(sema_value_runtime_should_be_constant(module, value_decl->initializer->slice,
                runtime));
            hir_set_var_initializer(module->hir, value_decl->sema.var_id, sema_const_to_hir(constant));
            return true;
        }
        case AST_VALUE_DECL_CONST: return true;
        case AST_VALUE_DECL_FINAL: UNREACHABLE;
    }
    UNREACHABLE;
}

bool sema_module_stage_emit_local(SemaModule *module, AstValueDecl *value_decl) {
    if (value_decl->global) {
        sema_module_err(module, value_decl->info->name, "local value cannot be global");
        return false;
    }
    if (value_decl->info->is_public) {
        sema_module_err(module, value_decl->info->name, "local value cannot be public");
        return false;
    }
    AstValueInfo *info = value_decl->info;

    if (info->kind == AST_VALUE_DECL_CONST) {
        if (!value_decl->initializer) {
            sema_module_err(module, value_decl->info->name, "constant value must be initialized!");
            return false;
        }
        SemaExprOutput output = sema_expr_output_new(module->mempool);
        SemaValueRuntime *runtime = NOT_NULL(sema_value_decl_get_initializer(module, value_decl, &output));
        SemaConst *constant = NOT_NULL(sema_value_runtime_should_be_constant(module, value_decl->initializer->slice,
            runtime));
        sema_module_push_decl(module, info->name, sema_decl_new(module->mempool, module,
            sema_value_new_runtime_const(module->mempool, constant)));
        return true;
    }

    SemaExprOutput tmp;
    SemaExprOutput *output = NULL;
    if (value_decl->initializer) {
        tmp = sema_expr_output_new(module->mempool);
        output = &tmp;
        NOT_NULL(sema_value_decl_get_initializer(module, value_decl, output));
    }

    if (!value_decl->sema.type) {
        sema_module_err(module, value_decl->info->name,
            "unable to determine value type, specify type explicitly or set initializer");
        return NULL;
    }

    HirLocalId local_id = hir_fun_add_local(module->hir, module->ss->func_id, hir_func_local_new(
        sema_type_hir_id(value_decl->sema.type), ast_value_kind_to_hir(info->kind)));
    sema_module_push_decl(module, info->name, sema_decl_new(module->mempool, module,
        sema_value_new_runtime_local(module->mempool, ast_value_kind_to_sema(info->kind), value_decl->sema.type,
            local_id)));

    switch (info->kind) {
        case AST_VALUE_DECL_VAR:
            sema_ss_append_stmt(module->ss, hir_stmt_new_decl_var(local_id));
            if (output) {
                sema_ss_append_stmt(module->ss, hir_stmt_new_store(
                    hir_expr_new(vec_create_in(module->mempool, hir_expr_step_new_get_local(local_id))),
                    sema_expr_output_collect(output)
                ));
            }
            return true;
        case AST_VALUE_DECL_FINAL:
            if (!output) {
                sema_module_err(module, value_decl->info->name, "final value must be initialized!");
                return false;
            }
            sema_ss_append_stmt(module->ss, hir_stmt_new_init_final(local_id, sema_expr_output_collect(output)));
            return true;
        case AST_VALUE_DECL_CONST: UNREACHABLE;
    }
    UNREACHABLE;
}

bool sema_module_stage_fill_value(SemaModule *module, AstValueDecl *value_decl) {
    AstValueInfo *info = value_decl->info;
    value_decl->sema.type = info->explicit_type ? NOT_NULL(sema_module_type(module, info->explicit_type)) : NULL;
    if (sema_module_is_global_scope(module)) {
        return sema_module_stage_fill_global(module, value_decl);
    } else {
        return true;
    }
}

bool sema_module_stage_emit_value(SemaModule *module, AstValueDecl *value_decl) {
    if (sema_module_is_global_scope(module)) {
        return sema_module_stage_emit_global(module, value_decl);
    } else {
        return sema_module_stage_emit_local(module, value_decl);
    }
}
