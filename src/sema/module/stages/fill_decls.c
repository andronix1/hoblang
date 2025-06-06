#include "fill_decls.h"
#include "ast/node.h"
#include "core/assert.h"
#include "core/mempool.h"
#include "core/null.h"
#include "core/vec.h"
#include "ir/api/ir.h"
#include "ir/func.h"
#include "sema/module/api/type.h"
#include "sema/module/ast/type.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
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
                    ir_init_extern(module->ir, ir_add_decl(module->ir),
                        ir_extern_new(IR_EXTERN_FUNC, info->fun->name,
                            sema_type_ir_id(type)));
                    return true;
                }
                case AST_EXTERNAL_DECL_VALUE:
                    TODO;
            }
            UNREACHABLE;
        }
        case AST_NODE_FUN_DECL: {
            AstFunDecl *info = &node->fun_decl;
            SemaType *type = NOT_NULL(ast_func_info_type(module, info->info));
            IrMutability *args_mut = vec_new_in(module->mempool, IrMutability);
            vec_resize(args_mut, vec_len(type->function.args));
            for (size_t i = 0; i < vec_len(args_mut); i++) {
                args_mut[i] = IR_MUTABLE;
            }
            info->sema.func_id = ir_init_func(module->ir, args_mut, ir_add_decl(module->ir),
                ir_func_new(sema_type_ir_id(type)));
            return true;
        }
        case AST_NODE_VALUE_DECL:
            if (sema_module_is_global_scope(module)) {
                // TODO
                sema_module_err(module, node->slice, "global values are not supported now :(");
                return false;
            }
            TODO;
        case AST_NODE_IMPORT:
        case AST_NODE_STMT:
            TODO;
    }
    UNREACHABLE;
}
