#pragma once

#include "core/mempool.h"
#include "ir/api/func.h" 
#include "ir/api/stmt/stmt.h"
#include "ir/stmt/stmt.h"
#include "sema/module/api/decl.h"
#include "sema/module/api/module.h"
#include "sema/module/api/type.h"

typedef struct {
    IrLoopId id;
    bool is_labeled;
    Slice label;
} SemaLoop;

static inline SemaLoop sema_loop_new(IrLoopId id) {
    SemaLoop loop = {
        .id = id,
        .is_labeled = false,
    };
    return loop;
}

static inline SemaLoop sema_loop_new_labeled(IrLoopId id, Slice label) {
    SemaLoop loop = {
        .id = id,
        .is_labeled = true,
        .label = label
    };
    return loop;
}

typedef struct {
    SemaLoop *loop;
    SemaDecl **decls_map;
    IrStmt **stmts;
    IrCode **defers;
    bool breaks;
} SemaScope;

typedef struct SemaScopeStack {
    SemaScope *scopes;
    IrFuncId func_id;
    SemaType *returns;
} SemaScopeStack;

SemaScopeStack *sema_scope_stack_new(Mempool *mempool, IrFuncId func_id, SemaType *returns);

IrLoopId *sema_ss_labeled_loop(SemaScopeStack *ss, Slice label);
IrLoopId *sema_ss_top_loop(SemaScopeStack *ss);
void sema_ss_push_defer(SemaScopeStack *ss, IrCode *code);
void sema_ss_push_scope(SemaScopeStack *ss, SemaLoop *loop, Mempool *mempool);
void sema_ss_pop_scope(SemaScopeStack *ss);
void sema_ss_append_stmt(SemaScopeStack *ss, IrStmt *stmt);
void sema_ss_append_body(SemaScopeStack *ss, IrCode *code);
IrStmt **sema_ss_get_stmts(SemaScopeStack *ss);

void sema_ss_push_decl(SemaModule *module, SemaScopeStack *ss, Slice name, SemaDecl *decl);
SemaDecl *sema_ss_resolve_decl(SemaScopeStack *ss, Slice name);
