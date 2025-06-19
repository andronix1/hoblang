#pragma once

#include "core/mempool.h"
#include "hir/api/func.h" 
#include "hir/api/code.h"
#include "sema/module/api/decl.h"
#include "sema/module/api/module.h"
#include "sema/module/api/type.h"

typedef struct {
    HirLoopId id;
    OptSlice label;
} SemaLoop;

static inline SemaLoop sema_loop_new(HirLoopId id, OptSlice label) {
    SemaLoop loop = {
        .id = id,
        .label = label,
    };
    return loop;
}

typedef struct {
    SemaLoop *loop;
    SemaDecl **decls_map;
    HirStmt *stmts;
    HirCode **defers;
    bool breaks;
} SemaScope;

typedef struct SemaScopeStack {
    SemaScope *scopes;
    HirFuncId func_id;
    SemaType *returns;
} SemaScopeStack;

SemaScopeStack *sema_scope_stack_new(Mempool *mempool, HirFuncId func_id, SemaType *returns);

HirLoopId *sema_ss_labeled_loop(SemaScopeStack *ss, Slice label);
HirLoopId *sema_ss_top_loop(SemaScopeStack *ss);
void sema_ss_push_defer(SemaScopeStack *ss, HirCode *code);
void sema_ss_push_scope(SemaScopeStack *ss, SemaLoop *loop, Mempool *mempool);
void sema_ss_pop_scope(SemaScopeStack *ss);
void sema_ss_append_stmt(SemaScopeStack *ss, HirStmt stmt);
void sema_ss_append_body(SemaScopeStack *ss, HirCode *code);
HirStmt *sema_ss_get_stmts(SemaScopeStack *ss);

void sema_ss_push_decl(SemaModule *module, SemaScopeStack *ss, Slice name, SemaDecl *decl);
SemaDecl *sema_ss_resolve_decl(SemaScopeStack *ss, Slice name);
