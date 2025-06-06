#include "fill_decls.h"
#include "ast/node.h"
#include "core/assert.h"
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
#include "sema/module/type.h"
#include "sema/module/value.h"
#include <stdio.h>

static inline SemaType *ast_func_info_type(SemaModule *module, AstFunInfo *info) {
    SemaType **args = vec_new_in(module->mempool, SemaType*);
    vec_resize(args, vec_len(info->args));
    for (size_t i = 0; i < vec_len(info->args); i++) {
        args[i] = NOT_NULL(sema_module_type(module, info->args->type));
    }
    SemaType *returns = info->returns ?
        NOT_NULL(sema_module_type(module, info->returns)) :
        sema_type_new_void(module);
    return sema_type_new_function(module, args, returns);
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

bool sema_module_fill_node_decls(SemaModule *module, AstNode *node) {
    switch (node->kind) {
        case AST_NODE_TYPE_DECL: {
            AstTypeDecl *info = &node->type_decl;
            SemaType *type = sema_type_new_alias(module->mempool,
                NOT_NULL(sema_module_type(module, node->type_decl.type)),
                sema_type_alias_new(module->mempool, info->sema.type_id)
            );
            ir_set_type_record(module->ir, sema_module_get_type_id(module, info->sema.type_id),
                sema_type_ir_id(type));
            return true;
        }
        case AST_NODE_EXTERNAL_DECL: {
            AstExternalDecl *info = &node->external_decl;
            switch (info->kind) {
                case AST_EXTERNAL_DECL_FUN: {
                    SemaType *type = NOT_NULL(ast_func_info_type(module, info->fun));
                    IrDeclId decl_id = ir_add_decl(module->ir);
                    ir_init_extern(module->ir, decl_id,
                        ir_extern_new(IR_EXTERN_FUNC,
                            info->has_alias ?
                                info->alias :
                                info->fun->name,
                            sema_type_ir_id(type)));
                    sema_module_push_decl(module, info->fun->name, sema_decl_new(
                        module->mempool, sema_value_new_runtime_global(module->mempool, 
                            type, decl_id)));
                    return true;
                }
                case AST_EXTERNAL_DECL_VALUE:
                    TODO;
            }
            UNREACHABLE;
        }
        case AST_NODE_FUN_DECL: {
            AstFunDecl *info = &node->fun_decl;
            SemaType *type = info->sema.type =
                NOT_NULL(ast_func_info_type(module, info->info));
            IrMutability *args_mut = vec_new_in(module->mempool, IrMutability);
            vec_resize(args_mut, vec_len(type->function.args));
            for (size_t i = 0; i < vec_len(args_mut); i++) {
                args_mut[i] = IR_MUTABLE;
            }
            IrTypeId type_id = sema_type_ir_id(type);
            info->sema.func_id = ir_init_func(module->ir, args_mut, ir_add_decl(module->ir),
                info->global ? 
                    ir_func_new_global(
                        info->global->has_alias ?
                            info->global->alias :
                            info->info->name, type_id
                        ) : ir_func_new(type_id));
            return true;
        }
        case AST_NODE_VALUE_DECL: {
            if (sema_module_is_global_scope(module)) {
                // TODO
                sema_module_err(module, node->slice, "global values are not supported now :(");
                return false;
            }
            AstValueInfo *info = node->value_decl.info;
            if (!info->explicit_type) {
                // TODO
                sema_module_err(module, node->slice, "type detection is not implemented now :(");
                return false;
            }
            SemaType *type = sema_module_type(module, info->explicit_type);
            IrLocalId local_id = ir_func_add_local(module->ir, module->ss->func_id,
                ir_func_local_new(ast_value_kind_to_ir(info->kind),
                    sema_type_ir_id(type)));
            node->value_decl.sema.type = type;
            node->value_decl.sema.local_id = local_id;
            sema_module_push_decl(module, info->name, sema_decl_new(
                module->mempool, sema_value_new_runtime_local(module->mempool,
                    type, local_id)));
            return true;
        }
        case AST_NODE_STMT: return true;
        case AST_NODE_IMPORT:
            TODO;
    }
    UNREACHABLE;
}
