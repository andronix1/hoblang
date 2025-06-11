#include "fill_decls.h"
#include "ast/node.h"
#include "ast/path.h"
#include "core/assert.h"
#include "core/keymap.h"
#include "core/mempool.h"
#include "core/null.h"
#include "core/vec.h"
#include "ir/api/ir.h"
#include "ir/func.h"
#include "ir/mut.h"
#include "sema/module/api/type.h"
#include "sema/module/ast/type.h"
#include "sema/module/decl.h"
#include "sema/module/module.h"
#include "sema/module/stmts/expr.h"
#include "sema/module/type.h"
#include "sema/module/value.h"
#include <stdio.h>

static inline SemaType *ast_func_info_type(SemaModule *module, AstFunInfo *info) {
    SemaType **args = vec_new_in(module->mempool, SemaType*);
    vec_reserve(args, vec_len(info->args) + info->ext.is);
    info->ext.sema.ext_type = NULL;
    if (info->ext.is) {
        AstType *of = ast_type_new_path(module->mempool, ast_path_new(module->mempool, vec_create_in(module->mempool,
            ast_path_segment_new_ident(info->ext.of))));
        SemaType *ext_of = info->ext.sema.ext_type = NOT_NULL(sema_module_type(module, of));
        vec_push(args, info->ext.by_ref ? sema_type_new_pointer(module, ext_of) : ext_of);
    }
    for (size_t i = 0; i < vec_len(info->args); i++) {
        vec_push(args, NOT_NULL(sema_module_type(module, info->args[i].type)));
    }
    SemaType *returns = info->returns ?
        NOT_NULL(sema_module_type(module, info->returns)) :
        sema_type_new_void(module);
    return sema_type_new_function(module, args, returns);
}

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

static inline void sema_module_push_fun_info_decl(SemaModule *module, AstFunInfo *info, SemaDecl *decl) {
    if (info->ext.is) {
        SemaType *ext_type = info->ext.sema.ext_type;
        if (!ext_type->aliases) {
            sema_module_err(module, info->ext.of, "only type aliases can be extended");
            return;
        }
        if (keymap_insert((*vec_top(ext_type->aliases))->decls_map, info->name, decl)) {
            sema_module_err(module, info->name, "`$S` already declared in type alias", info->name);
            return;
        }
    } else {
        sema_module_push_decl(module, info->name, decl);
    }
}

bool sema_module_fill_node_decls(SemaModule *module, AstNode *node) {
    switch (node->kind) {
        case AST_NODE_TYPE_DECL: {
            AstTypeDecl *info = &node->type_decl;
            SemaType *source_type = module->types[info->sema.type_id].type =
                NOT_NULL(sema_module_type(module, node->type_decl.type));
            sema_type_new_alias(module->mempool, source_type, sema_type_alias_new(module->mempool, info->sema.type_id));
            ir_set_type_record(module->ir, module->types[info->sema.type_id].id, sema_type_ir_id(source_type));
            return true;
        }
        case AST_NODE_EXTERNAL_DECL: {
            AstExternalDecl *info = &node->external_decl;
            switch (info->kind) {
                case AST_EXTERNAL_DECL_FUN: {
                    SemaType *type = NOT_NULL(ast_func_info_type(module, info->fun));
                    IrDeclId decl_id = ir_add_decl(module->ir);
                    ir_init_extern(module->ir, decl_id, ir_extern_new(IR_EXTERN_FUNC,
                        info->has_alias ? info->alias : info->fun->name, sema_type_ir_id(type)));
                    sema_module_push_fun_info_decl(module, info->fun, sema_decl_new(module->mempool,
                        sema_value_new_runtime_global(module->mempool, SEMA_RUNTIME_FINAL, type, decl_id)));
                    return true;
                }
                case AST_EXTERNAL_DECL_VALUE:
                    TODO;
            }
            UNREACHABLE;
        }
        case AST_NODE_FUN_DECL: {
            AstFunDecl *info = &node->fun_decl;
            SemaType *type = info->sema.type = NOT_NULL(ast_func_info_type(module, info->info));
            IrMutability *args_mut = vec_new_in(module->mempool, IrMutability);
            vec_resize(args_mut, vec_len(type->function.args));
            for (size_t i = 0; i < vec_len(type->function.args); i++) {
                args_mut[i] = i == 0 && info->info->ext.is ? IR_IMMUTABLE : IR_MUTABLE;
            }
            IrDeclId decl_id = ir_add_decl(module->ir);
            sema_module_push_fun_info_decl(module, info->info, sema_decl_new(module->mempool,
                sema_value_new_runtime_global(module->mempool, SEMA_RUNTIME_FINAL, type, decl_id)));
            IrTypeId type_id = sema_type_ir_id(type);
            info->sema.func_id = ir_init_func(module->ir, args_mut, decl_id, info->global ? 
                ir_func_new_global(info->global->has_alias ? info->global->alias : info->info->name, type_id) :
                ir_func_new(type_id));
            return true;
        }
        case AST_NODE_VALUE_DECL: {
            if (sema_module_is_global_scope(module)) {
                // TODO
                sema_module_err(module, node->slice, "global values are not supported now :(");
                return false;
            }
            AstValueInfo *info = node->value_decl.info;
            assert(node->value_decl.initializer);
            SemaType *type = info->explicit_type ? sema_module_type(module, info->explicit_type) : NULL;
            SemaExprOutput output = sema_expr_output_new(module->mempool);
            SemaValueRuntime *value = NOT_NULL(sema_module_analyze_runtime_expr(module,
                node->value_decl.initializer, sema_expr_ctx_new(&output, type)));
            if (type && !sema_type_eq(type, value->type)) {
                sema_module_err(module, node->value_decl.initializer->slice, 
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
            node->value_decl.sema.type = type;
            node->value_decl.sema.local_id = local_id;
            sema_module_push_decl(module, info->name, sema_decl_new(
                module->mempool, sema_value_new_runtime_local(module->mempool,
                    ast_value_kind_to_sema(info->kind), type, local_id)));
            switch (info->kind) {
                case AST_VALUE_DECL_FINAL: {
                    sema_ss_append_stmt(module->ss, ir_stmt_new_init_final(module->mempool,
                        node->value_decl.sema.local_id,
                        ir_expr_new(output.steps)
                    ));
                    return true;
                }
                case AST_VALUE_DECL_VAR: {
                    sema_ss_append_stmt(module->ss, ir_stmt_new_decl_var(module->mempool,
                        node->value_decl.sema.local_id
                    ));
                    sema_ss_append_stmt(module->ss, ir_stmt_new_store(module->mempool,
                        ir_expr_new(vec_create_in(module->mempool, 
                            ir_expr_step_new_get_local(node->value_decl.sema.local_id)
                        )),
                        ir_expr_new(output.steps)
                    ));
                    return true;
                }
                case AST_VALUE_DECL_CONST:
                    TODO;
            }
            UNREACHABLE;
        }
        case AST_NODE_STMT: return true;
        case AST_NODE_IMPORT:
            TODO;
    }
    UNREACHABLE;
}
