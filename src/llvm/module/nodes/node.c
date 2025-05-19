#include "node.h"
#include "ast/node.h"
#include "core/assert.h"
#include "core/mempool.h"
#include "llvm/module/module.h"
#include "llvm/module/nodes/body.h"
#include "llvm/module/nodes/expr.h"
#include "llvm/module/nodes/stmt.h"
#include "llvm/module/nodes/type.h"
#include "sema/module/module.h"
#include <llvm-c/Core.h>
#include <llvm-c/Types.h>

static void llvm_add_function(LlvmModule *module, AstFunInfo *info, Slice *name) {
    const char *function_name = "";
    if (name) {
        function_name = mempool_slice_to_cstr(module->mempool, *name);
    }
    info->sema.decl->llvm.value = LLVMAddFunction(module->module, function_name, llvm_decl_type(module, info->sema.decl));
}

void llvm_module_read_node(LlvmModule *module, AstNode *node) {
    switch (node->kind) {
        case AST_NODE_STMT:
        case AST_NODE_IMPORT:
        case AST_NODE_TYPE_DECL:
            return;
        case AST_NODE_FUN_DECL:
            llvm_add_function(module, node->fun_decl.info, node->fun_decl.global ?
                (node->fun_decl.global->has_alias ? &node->fun_decl.global->alias :
                    &node->fun_decl.info->name) : NULL);
            return;
        return;
        case AST_NODE_EXTERNAL_DECL:
            switch (node->external_decl.kind) {
                case AST_EXTERNAL_DECL_VALUE:
                    TODO;
                case AST_EXTERNAL_DECL_FUN:
                    llvm_add_function(module, node->external_decl.fun,
                        node->external_decl.has_alias ? &node->external_decl.alias :
                            &node->external_decl.fun->name);
                    return;
            }
            UNREACHABLE;
        case AST_NODE_VALUE_DECL:
            if (node->value_decl.sema.is_global) {
                node->value_decl.sema.decl->llvm.value = LLVMAddGlobal(module->module,
                    llvm_decl_type(module, node->value_decl.sema.decl), "");
            } else {
                if (node->value_decl.info->kind == AST_VALUE_DECL_VAR) {
                    node->value_decl.sema.decl->llvm.value = llvm_alloca(module,
                        llvm_decl_type(module, node->value_decl.sema.decl));
                }
            }
            return;
    }
    UNREACHABLE;
}

void llvm_module_emit_node(LlvmModule *module, AstNode *node) {
    switch (node->kind) {
        case AST_NODE_FUN_DECL: {
            LLVMValueRef func = node->fun_decl.info->sema.decl->llvm.value;
            LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(module->context, func, "");
            LLVMPositionBuilderAtEnd(module->builder, entry);
            LlvmState old_state = llvm_switch_state(module, llvm_state_new(func, entry, entry));
            size_t offset = node->fun_decl.info->ext.is;
            if (node->fun_decl.info->ext.is) {
                node->fun_decl.info->ext.sema.self->llvm.value = LLVMGetParam(func, 0);
            }
            for (size_t i = 0; i < vec_len(node->fun_decl.info->args); i++) {
                LLVMValueRef value = node->fun_decl.info->args[i].sema.decl->llvm.value =
                    llvm_alloca(module, llvm_decl_type(module, node->fun_decl.info->args->sema.decl));
                LLVMBuildStore(module->builder, LLVMGetParam(func, i + offset), value);
            }
            llvm_emit_body(module, node->fun_decl.body);
            if (!node->fun_decl.body->sema.finished) {
                LLVMBuildRetVoid(module->builder);
            }
            llvm_switch_state(module, old_state);
            return;
        }
        case AST_NODE_IMPORT:
        case AST_NODE_TYPE_DECL:
        case AST_NODE_EXTERNAL_DECL:
            return;
        case AST_NODE_STMT:
            llvm_emit_stmt(module, node->stmt);
            return;
        case AST_NODE_VALUE_DECL: {
            AstExpr *init = node->value_decl.initializer;
            if (!init) return;
            LLVMValueRef value = node->value_decl.sema.decl->llvm.value;
            LLVMValueRef init_val = llvm_emit_expr_get(module, init);
            if (node->value_decl.sema.is_global) {
                LLVMSetInitializer(value, init_val);
            } else {
                if (node->value_decl.info->kind == AST_VALUE_DECL_VAR) {
                    LLVMBuildStore(module->builder, init_val, value);
                } else {
                    node->value_decl.sema.decl->llvm.value = init_val;
                }
            }
            return;
        }
    }
    UNREACHABLE;
}
