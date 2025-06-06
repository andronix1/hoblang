#include "stmt.h"
#include "core/assert.h"
#include "core/null.h"
#include "ir/stmt/expr.h"
#include "ir/stmt/stmt.h"
#include "sema/module/api/value.h"
#include "sema/module/module.h"
#include "sema/module/scope.h"
#include "sema/module/stmts/expr.h"
#include "sema/module/stmts/if.h"

bool sema_module_emit_stmt(SemaModule *module, AstStmt *stmt) {
    switch (stmt->kind) {
        case AST_STMT_EXPR: {
            SemaExprOutput output = sema_expr_output_new(module->mempool);
            NOT_NULL(sema_module_analyze_expr(module, stmt->expr, sema_expr_ctx_new(&output, NULL)));
            sema_ss_append_stmt(module->ss, ir_stmt_new_expr(module->mempool,
                    ir_expr_new(output.steps)));
            return true;
        }
        case AST_STMT_RETURN: {
            assert(stmt->ret.value);

            SemaExprOutput output = sema_expr_output_new(module->mempool);
            SemaValueRuntime *runtime = NOT_NULL(sema_module_analyze_runtime_expr(module,
                stmt->ret.value, sema_expr_ctx_new(&output, module->ss->returns)));

            if (!sema_type_eq(module->ss->returns, runtime->type)) {
                sema_module_err(module, stmt->expr->slice, "expected to return $t, got $t",
                    module->ss->returns, runtime->type);
                return false;
            }
            sema_ss_append_stmt(module->ss, ir_stmt_new_ret(module->mempool,
                    ir_expr_new(output.steps)));
            return true;
        }
        case AST_STMT_IF:
            return sema_module_emit_if(module, &stmt->if_else);
        case AST_STMT_ASSIGN:
        case AST_STMT_WHILE:
            TODO;
    }
    UNREACHABLE;
}
