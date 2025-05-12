#include "node.h"
#include "ast/node.h"
#include "ast/expr.h"
#include "core/assert.h"
#include "core/mempool.h"
#include "core/null.h"
#include "core/vec.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"
#include "sema/module/module.h"
#include "sema/module/nodes/body.h"
#include "sema/module/nodes/expr.h"
#include "sema/module/nodes/type.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

static inline void sema_module_push_fun_info(SemaModule *module, AstFunInfo *info) {
    info->sema.type = NULL;
    SemaType *returns = info->returns ?
       RET_ON_NULL(sema_module_analyze_type(module, info->returns)) :
       sema_type_new_primitive_void(module->mempool);
    SemaType **args = vec_new_in(module->mempool, SemaType*);
    for (size_t i = 0; i < vec_len(info->args); i++) {
        vec_push(args, RET_ON_NULL(sema_module_analyze_type(module, info->args[i].type)));
    }
    SemaType *type = sema_type_new_function(module->mempool, args, returns);
    SemaDecl decl = sema_decl_new(module, info->is_local, sema_value_new_final(module->mempool, type));
    info->sema.type = type;
    sema_module_push_decl(module, info->name, decl);
}

void sema_module_read_node(SemaModule *module, AstNode *node) {
    switch (node->kind) {
        case AST_NODE_TYPE_DECL: {
            SemaType *type = sema_type_new_alias(module->mempool, module,
                RET_ON_NULL(sema_module_analyze_type(module, node->type_decl.type)));
            sema_module_push_decl(module, node->type_decl.name, sema_decl_new(module, node->type_decl.is_local,
                sema_value_new_type(module->mempool, type)));
            return;
        }
        case AST_NODE_FUN_DECL: {
            sema_module_push_fun_info(module, node->fun_decl.info);
            return;
        }
        case AST_NODE_VALUE_DECL:
            if (!node->value_decl.info->explicit_type) {
                sema_module_err(module, node->value_decl.info->name, "type of declaration must be specified (TODO)");
                return;
            }
            SemaType *type = RET_ON_NULL(sema_module_analyze_type(module, node->value_decl.info->explicit_type));
            switch (node->value_decl.info->kind) {
                case AST_VALUE_DECL_VAR:
                    sema_module_push_decl(module, node->value_decl.info->name,
                        sema_decl_new(module, node->value_decl.info->is_local,
                            sema_value_new_var(module->mempool, type)
                        ));
                    return;
                case AST_VALUE_DECL_FINAL:
                    sema_module_push_decl(module, node->value_decl.info->name,
                        sema_decl_new(module, node->value_decl.info->is_local,
                            sema_value_new_final(module->mempool, type)
                        ));
                    return;
                case AST_VALUE_DECL_CONST:
                    TODO;
            }
            UNREACHABLE;
        case AST_NODE_EXTERNAL_DECL: {
            switch (node->external_decl.kind) {
                case AST_EXTERNAL_DECL_VALUE: {
                    AstValueInfo *info = node->external_decl.value;
                    if (!info->explicit_type) {
                        sema_module_err(module, info->name, "type of external declarations must be specified");
                        return;
                    }
                    SemaType *type = RET_ON_NULL(sema_module_analyze_type(module, info->explicit_type));
                    switch (info->kind) {
                        case AST_VALUE_DECL_VAR:
                            sema_module_push_decl(module, info->name, sema_decl_new(module, info->is_local,
                                sema_value_new_var(module->mempool, type)));
                            return;
                        case AST_VALUE_DECL_FINAL:
                            sema_module_push_decl(module, info->name, sema_decl_new(module, info->is_local,
                                sema_value_new_final(module->mempool, type)));
                            return;
                        case AST_VALUE_DECL_CONST:
                            sema_module_err(module, info->name, "constant cannot be external");
                            return;
                    }
                    UNREACHABLE;
                }
                case AST_EXTERNAL_DECL_FUN:
                    sema_module_push_fun_info(module, node->external_decl.fun);
                    return;
            }
            UNREACHABLE;
        }
        case AST_NODE_STMT:
            return;
    }
    UNREACHABLE;
}

bool sema_module_analyze_node(SemaModule *module, AstNode *node) {
    switch (node->kind) {
        case AST_NODE_TYPE_DECL:
        case AST_NODE_VALUE_DECL:
        case AST_NODE_EXTERNAL_DECL:
            return false;
        case AST_NODE_FUN_DECL: {
            if (!node->fun_decl.info->sema.type) return false;
            SemaScopeStack ss = sema_ss_new(module, node->fun_decl.info->sema.type->function.returns);
            SemaScopeStack *old_ss = sema_module_ss_swap(module, &ss);
            bool breaks = sema_module_analyze_body(module, node->fun_decl.body);
            sema_module_ss_swap(module, old_ss);
            return breaks;
        }
        case AST_NODE_STMT:
            if (sema_module_is_global_scope(module)) {
                sema_module_err(module, node->slice, "cannot use statements in global scope");
                return false;
            }
            switch (node->stmt->kind) {
                case AST_STMT_EXPR:
                    NOT_NULL(sema_module_analyze_expr(module, node->stmt->expr, sema_expr_ctx_new(NULL)));
                    return false;
                case AST_STMT_RETURN: {
                    SemaType *returns = sema_module_returns(module);
                    SemaType *type = node->stmt->ret.value ?
                        sema_value_is_runtime(NOT_NULL(
                            sema_module_analyze_expr(module, node->stmt->ret.value, sema_expr_ctx_new(returns)))) :
                        sema_type_new_primitive_void(module->mempool);
                    if (!type) {
                        sema_module_err(module, node->stmt->ret.value->slice, "cannot return non-runtime value");
                        return true;
                    }
                    if (!sema_type_eq(type, returns)) {
                        sema_module_err(module, node->stmt->ret.value ?
                            node->stmt->ret.value->slice :
                            node->stmt->ret.slice, "expected to return $t but returns $t",
                            returns, type);
                    }
                    return true;
                }
            }
            UNREACHABLE;
    }
    UNREACHABLE;
}
