#include "value.h"
#include "core/assert.h"
#include "core/null.h"
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
        case AST_VALUE_DECL_CONST:
            TODO;
    }
    UNREACHABLE;
}

static inline IrMutability ast_value_kind_to_ir(AstValueDeclKind kind) {
    switch (kind) {
        case AST_VALUE_DECL_VAR:
            return IR_MUTABLE;
        case AST_VALUE_DECL_FINAL:
        case AST_VALUE_DECL_CONST:
            return IR_IMMUTABLE;
    }
    UNREACHABLE;
}

bool sema_module_stage_fill_value(SemaModule *module, AstValueDecl *value_decl) {
    if (sema_module_is_global_scope(module)) {
        // TODO
        sema_module_err(module, value_decl->info->name, "global values are not supported now :(");
        return false;
    }
    AstValueInfo *info = value_decl->info;
    assert(value_decl->initializer);
    SemaType *type = info->explicit_type ? sema_module_type(module, info->explicit_type) : NULL;
    SemaExprOutput output = sema_expr_output_new(module->mempool);
    SemaValueRuntime *value = NOT_NULL(sema_module_emit_runtime_expr(module,
        value_decl->initializer, sema_expr_ctx_new(&output, type)));
    if (type && !sema_type_eq(type, value->type)) {
        sema_module_err(module, value_decl->initializer->slice, 
            "trying to initialize declaration with exlicit type $t "
            "with expression of type $t", type, value->type);
        return false;
    }
    if (!type) {
        type = value->type;
    }
    IrLocalId local_id = ir_func_add_local(module->ir, module->ss->func_id,
        ir_func_local_new(ast_value_kind_to_ir(info->kind),
            sema_type_ir_id(type)));
    value_decl->sema.type = type;
    value_decl->sema.local_id = local_id;
    sema_module_push_decl(module, info->name, sema_decl_new(module->mempool,
        info->is_public ? NULL : module,
        sema_value_new_runtime_local(module->mempool,
            ast_value_kind_to_sema(info->kind), type, local_id)));
    switch (info->kind) {
        case AST_VALUE_DECL_FINAL: {
            sema_ss_append_stmt(module->ss, ir_stmt_new_init_final(module->mempool,
                value_decl->sema.local_id,
                sema_expr_output_collect(&output)
            ));
            return true;
        }
        case AST_VALUE_DECL_VAR: {
            sema_ss_append_stmt(module->ss, ir_stmt_new_decl_var(module->mempool,
                value_decl->sema.local_id
            ));
            sema_ss_append_stmt(module->ss, ir_stmt_new_store(module->mempool,
                ir_expr_new(vec_create_in(module->mempool, 
                    ir_expr_step_new_get_local(value_decl->sema.local_id)
                )),
                sema_expr_output_collect(&output)
            ));
            return true;
        }
        case AST_VALUE_DECL_CONST:
            TODO;
    }
    UNREACHABLE;
}
