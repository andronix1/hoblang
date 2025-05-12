#pragma once

#include "core/mempool.h"
#include "parser/api.h"
#include "sema/module/api.h"
#include "sema/module/api/decl_handle.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"

typedef struct SemaDeclHandle {
    SemaValue *value;
} SemaDeclHandle;

typedef struct {
    SemaModule *in;
    bool is_local;
    SemaDeclHandle *handle;
} SemaDecl;

typedef struct {
    SemaDecl *decls_map;
} SemaScope;

typedef struct {
    SemaScope *scopes;
    SemaType *returns;
} SemaScopeStack;

typedef struct SemaModule {
    Parser *parser;
    AstNode **nodes;
    Mempool *mempool;
    SemaDecl *decls_map;
    SemaScopeStack *ss;
} SemaModule;

SemaModule *sema_module_new(Parser *parser);
void sema_module_push_primitives(SemaModule *module);
SemaScopeStack *sema_module_ss_swap(SemaModule *module, SemaScopeStack *ss);
SemaDeclHandle *sema_module_push_decl(SemaModule *module, Slice name, SemaDecl decl);
SemaDeclHandle *sema_module_resolve_decl(const SemaModule *module, Slice name);
void sema_module_push_scope(SemaModule *module);
void sema_module_pop_scope(SemaModule *module);
SemaType *sema_module_returns(const SemaModule *module);
bool sema_module_is_global_scope(const SemaModule *module);
void sema_module_free(SemaModule *module);

SemaDecl sema_decl_new(SemaModule *module, bool is_local, SemaValue *value);

SemaScopeStack sema_ss_new(SemaModule *module, SemaType *returns);

void sema_module_err(SemaModule *module, Slice at, const char *fmt, ...);
