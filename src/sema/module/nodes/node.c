#include "node.h"
#include "ast/node.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "core/assert.h"
#include "core/keymap.h"
#include "core/mempool.h"
#include "core/null.h"
#include "core/path.h"
#include "core/vec.h"
#include "sema/module/alias.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"
#include "sema/module/module.h"
#include "sema/module/nodes/body.h"
#include "sema/module/nodes/expr.h"
#include "sema/module/nodes/exprs/binop.h"
#include "sema/module/nodes/generic.h"
#include "sema/module/nodes/stmts/if.h"
#include "sema/module/nodes/stmts/while.h"
#include "sema/module/nodes/type.h"
#include "sema/module/type.h"
#include "sema/module/value.h"
#include "sema/project.h"
#include <stdio.h>

static inline void sema_module_push_fun_info(SemaModule *module, AstFunInfo *info) {
    info->sema.decl = NULL;
    SemaType *returns = info->returns ?
       RET_ON_NULL(sema_module_analyze_type(module, info->returns)) :
       sema_type_new_primitive_void(module->mempool);
    SemaType **args = vec_new_in(module->mempool, SemaType*);
    SemaType *ext_type = NULL;
    if (info->ext.is) {
        ext_type = sema_value_is_type(RET_ON_NULL(sema_module_resolve_required_decl(module, info->ext.of))->value);
        if (!ext_type) {
            sema_module_err(module, info->ext.of, "$S is not a type", info->ext.of);
            return;
        }
        vec_push(args, info->ext.by_ref ?
            sema_type_new_pointer(module->mempool, ext_type) :
            ext_type);
    }
    for (size_t i = 0; i < vec_len(info->args); i++) {
        vec_push(args, RET_ON_NULL(sema_module_analyze_type(module, info->args[i].type)));
    }
    SemaType *type = sema_type_new_function(module->mempool, args, returns);
    SemaDecl decl = sema_decl_new(module, info->is_local, sema_value_new_final(module->mempool, type));
    info->sema.decl = decl.handle;
    if (!info->ext.is) {
        sema_module_push_decl(module, info->name, decl);
        return;
    }
    if (!ext_type->alias) {
        sema_module_err(module, info->ext.of, "$S is not alias, only aliases can be extended", info->ext.of);
        return;
    }
    if (keymap_insert(ext_type->alias->ext_map, info->name, sema_type_alias_ext_new_fun(decl, info->ext.by_ref))) {
        sema_module_err(module, info->name, "redeclaration of `%S`", info->name);
        return;
    }
}

void sema_module_read_node(SemaModule *module, AstNode *node) {
    switch (node->kind) {
        case AST_NODE_TYPE_DECL: {
            SemaTypeAlias *alias = sema_type_alias_new(module->mempool);
            if (node->type_decl.generics) {
                SemaGenericScopeHandle ghandle;
                sema_module_generic_setup(module, node->type_decl.generics, &ghandle);
                SemaType *type = sema_module_analyze_type(module, node->type_decl.type);
                sema_module_generic_clean(module, type, &ghandle);
                type = sema_type_new_alias(module->mempool, alias, RET_ON_NULL(type));
                sema_module_push_decl(module, node->type_decl.name, sema_decl_new(module, node->type_decl.is_local,
                    sema_value_new_generic(module->mempool, ghandle.generic)));
            } else {
                SemaType *type =  sema_type_new_alias(module->mempool, alias,
                    RET_ON_NULL(sema_module_analyze_type(module, node->type_decl.type)));
                sema_module_push_decl(module, node->type_decl.name, sema_decl_new(module, node->type_decl.is_local,
                    sema_value_new_type(module->mempool, type)));
            }
            node->type_decl.sema.alias = alias;
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
            AstExpr *init = node->value_decl.initializer;
            if (init) {
                SemaType *init_type = sema_value_is_runtime(RET_ON_NULL(sema_module_analyze_expr(module,
                    init, sema_expr_ctx_new(type))));
                if (!init_type) {
                    sema_module_err(module, init->slice, "initializer must be a runtime value");
                    return;
                }
                if (!sema_type_eq(init_type, type)) {
                    sema_module_err(module, init->slice, "cannot assign $t to value with explicit type $t", init_type, type);
                    return;
                }
            }
            node->value_decl.sema.is_global = sema_module_is_global_scope(module);
            switch (node->value_decl.info->kind) {
                case AST_VALUE_DECL_VAR:
                    node->value_decl.sema.decl = sema_module_push_decl(module, node->value_decl.info->name,
                        sema_decl_new(module, node->value_decl.info->is_local,
                            sema_value_new_var(module->mempool, type)
                        ));
                    return;
                case AST_VALUE_DECL_FINAL:
                    node->value_decl.sema.decl = sema_module_push_decl(module, node->value_decl.info->name,
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
        case AST_NODE_IMPORT: {
            SemaModule *import_module = RET_ON_NULL(sema_project_read(module->project,
                path_join_in(module->mempool, path_dirname_in(module->mempool, sema_module_path(module)),
                    mempool_slice_to_cstr(module->mempool, node->import.path))));
            sema_module_push_decl(module, node->import.alias, sema_decl_new(module,
                false, sema_value_new_module(module->mempool, import_module)));
            return;
        }
    }
    UNREACHABLE;
}

bool sema_module_analyze_node(SemaModule *module, AstNode *node) {
    switch (node->kind) {
        case AST_NODE_IMPORT:
        case AST_NODE_TYPE_DECL:
        case AST_NODE_VALUE_DECL:
        case AST_NODE_EXTERNAL_DECL:
            return false;
        case AST_NODE_FUN_DECL: {
            if (!node->fun_decl.info->sema.decl) return false;
            SemaType *type = sema_value_is_runtime(node->fun_decl.info->sema.decl->value);
            assert(type);
            SemaScopeStack ss = sema_ss_new(module, type->function.returns);
            SemaScopeStack *old_ss = sema_module_ss_swap(module, &ss);
            sema_module_push_scope(module);
            size_t offset = node->fun_decl.info->ext.is;
            if (node->fun_decl.info->ext.is) {
                node->fun_decl.info->ext.sema.self = sema_module_push_decl(module,
                    node->fun_decl.info->ext.self_name, sema_decl_new(module, false,
                        sema_value_new_final(module->mempool, type->function.args[0])));
            }
            for (size_t i = offset; i < vec_len(type->function.args); i++) {
                node->fun_decl.info->args[i - offset].sema.decl = sema_module_push_decl(module,
                    node->fun_decl.info->args[i - offset].name, sema_decl_new(module, false,
                        sema_value_new_var(module->mempool, type->function.args[i])));
            }
            bool breaks = sema_module_analyze_body(module, node->fun_decl.body);
            if (!breaks && !sema_type_is_void(type->function.returns)) {
                sema_module_err(module, node->fun_decl.info->name, "body was not broke but $t expected to return",
                    type->function.returns);
            }
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
                case AST_STMT_ASSIGN: {
                    SemaType *dst_type = sema_value_is_var(NOT_NULL(sema_module_analyze_expr(module,
                        node->stmt->assign.dst, sema_expr_ctx_new(NULL))));
                    if (!dst_type) {
                        sema_module_err(module, node->stmt->assign.dst->slice, "cannot assign to non-var expression");
                    }
                    SemaType *what_type = sema_value_is_runtime(NOT_NULL(sema_module_analyze_expr(module,
                        node->stmt->assign.what, sema_expr_ctx_new(dst_type))));
                    if (!what_type) {
                        sema_module_err(module, node->stmt->assign.what->slice,
                            "cannot assign non-runtime expression");
                        return false;
                    }
                    if (!dst_type) return false;
                    if (!sema_type_eq(dst_type, what_type)) {
                        sema_module_err(module, node->stmt->assign.what->slice,
                            "cannot assign $t value to $t variable", what_type, dst_type);
                        return false;
                    }
                    if (node->stmt->assign.short_assign.is) {
                        sema_module_analyze_binop(module, what_type, dst_type, &node->stmt->assign.short_assign.kind);
                    }
                    return false;
                }
                case AST_STMT_IF: return sema_module_analyze_if(module, &node->stmt->if_else);
                case AST_STMT_WHILE: return sema_module_analyze_while(module, &node->stmt->while_loop);
            }
            UNREACHABLE;
    }
    UNREACHABLE;
}
