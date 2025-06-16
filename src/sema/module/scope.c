#include "scope.h"
#include "core/keymap.h"
#include "core/mempool.h"
#include "core/slice.h"
#include "core/vec.h"
#include "sema/module/api/module.h"
#include "sema/module/module.h"
#include <stdio.h>

SemaScopeStack *sema_scope_stack_new(Mempool *mempool, IrFuncId func_id, SemaType *returns)
    MEMPOOL_CONSTRUCT(SemaScopeStack,
        out->returns = returns;
        out->func_id = func_id;
        out->scopes = vec_new_in(mempool, SemaScope);
    )

static inline SemaScope sema_scope_new(Mempool *mempool, SemaLoop *loop) {
    SemaScope scope = {
        .loop = loop,
        .decls_map = keymap_new_in(mempool, SemaDecl*),
        .stmts = vec_new_in(mempool, IrStmt*),
        .defers = vec_new_in(mempool, IrCode*),
    };
    return scope;
}

IrLoopId *sema_ss_labeled_loop(SemaScopeStack *ss, Slice label) {
    for (ssize_t i = (ssize_t)vec_len(ss->scopes) - 1; i >= 0; i--) {
        SemaLoop *loop = ss->scopes[i].loop;
        if (!loop) continue;
        if (loop->is_labeled && slice_eq(label, loop->label)) {
            return &loop->id;
        }
    }
    return NULL;
}

IrLoopId *sema_ss_top_loop(SemaScopeStack *ss) {
    for (ssize_t i = (ssize_t)vec_len(ss->scopes) - 1; i >= 0; i--) {
        SemaLoop *loop = ss->scopes[i].loop;
        if (loop) return &loop->id;
    }
    return NULL;
}

void sema_ss_push_scope(SemaScopeStack *ss, SemaLoop *loop, Mempool *mempool) {
    vec_push(ss->scopes, sema_scope_new(mempool, loop));
}

void sema_ss_pop_scope(SemaScopeStack *ss) {
    vec_pop(ss->scopes);
}

void sema_ss_append_stmt(SemaScopeStack *ss, IrStmt *stmt) {
    assert(vec_len(ss->scopes));
    vec_push(vec_top(ss->scopes)->stmts, stmt);
}

IrStmt **sema_ss_get_stmts(SemaScopeStack *ss) {
    assert(vec_len(ss->scopes));
    return vec_top(ss->scopes)->stmts;
}

void sema_ss_push_decl(SemaModule *module, SemaScopeStack *ss, Slice name, SemaDecl *decl) {
    assert(vec_len(ss->scopes));
    SemaScope *top = vec_top(ss->scopes);
    if (keymap_insert(top->decls_map, name, decl)) {
        sema_module_err(module, name, "`$S` already declared", name);
    }
}

void sema_ss_push_defer(SemaScopeStack *ss, IrCode *code) {
    assert(vec_len(ss->scopes) > 0);
    vec_push(vec_top(ss->scopes)->defers, code);
}

void sema_ss_append_body(SemaScopeStack *ss, IrCode *code) {
    assert(vec_len(ss->scopes));
    vec_extend(vec_top(ss->scopes)->stmts, code->stmts);
}

SemaDecl *sema_ss_resolve_decl(SemaScopeStack *ss, Slice name) {
    for (ssize_t i = (ssize_t)vec_len(ss->scopes) - 1; i >= 0; i--) {
        SemaDecl **decl = keymap_get(ss->scopes[i].decls_map, name);
        if (decl) {
            return *decl;
        }
    }
    return NULL;
}
