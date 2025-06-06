#pragma once

#include "core/mempool.h"
#include "ir/api/func.h"
#include "ir/api/stmt/stmt.h"
#include "sema/module/api/decl.h"
#include "sema/module/api/module.h"
#include "sema/module/api/type.h"

typedef struct {
    SemaDecl **decls_map;
    IrStmt **stmts;
} SemaScope;

typedef struct SemaScopeStack {
    SemaScope *scopes;
    IrFuncId func_id;
    SemaType *returns;
} SemaScopeStack;

SemaScopeStack *sema_scope_stack_new(Mempool *mempool, IrFuncId func_id, SemaType *returns);

void sema_ss_push_scope(SemaScopeStack *ss, Mempool *mempool);
void sema_ss_pop_scope(SemaScopeStack *ss);
void sema_ss_append_stmt(SemaScopeStack *ss, IrStmt *stmt);
IrStmt **sema_ss_get_stmts(SemaScopeStack *ss);

void sema_ss_push_decl(SemaModule *module, SemaScopeStack *ss, Slice name, SemaDecl *decl);
SemaDecl *sema_ss_resolve_decl(SemaScopeStack *ss, Slice name);
