#include "stmt.h"
#include "core/assert.h"
#include "sema/module/module.h"
#include "sema/module/stmts/assign.h"
#include "sema/module/stmts/defer.h"
#include "sema/module/stmts/expr.h"
#include "sema/module/stmts/if.h"
#include "sema/module/stmts/loop_control.h"
#include "sema/module/stmts/return.h"
#include "sema/module/stmts/while.h"

bool sema_module_emit_stmt(SemaModule *module, AstStmt *stmt) {
    switch (stmt->kind) {
        case AST_STMT_EXPR: return sema_module_emit_stmt_expr(module, stmt->expr);
        case AST_STMT_DEFER: return sema_module_emit_stmt_defer(module, &stmt->defer);
        case AST_STMT_RETURN: return sema_module_emit_stmt_return(module, &stmt->ret);
        case AST_STMT_IF: return sema_module_emit_stmt_if(module, &stmt->if_else);
        case AST_STMT_ASSIGN: return sema_module_emit_stmt_assign(module, &stmt->assign);
        case AST_STMT_WHILE: return sema_module_emit_stmt_while(module, &stmt->while_loop);
        case AST_STMT_CONTINUE: return sema_module_emit_stmt_continue(module, &stmt->continue_loop);
        case AST_STMT_BREAK: return sema_module_emit_stmt_break(module, &stmt->break_loop);
    }
    UNREACHABLE;
}
