#pragma once

#include "ast/api/node.h"
#include "core/mempool.h"
#include "core/slice.h"
#include "hir/api/hir.h"
#include "parser/api.h"
#include "sema/api/project.h"
#include "sema/module/api/decl.h"
#include "sema/module/api/type.h"
#include "sema/module/api/decl.h"
#include "sema/module/std.h"
#include "sema/module/scope.h"

typedef struct SemaTypeInfo {
    SemaType *type;
    HirTypeId id;
} SemaTypeInfo;

typedef struct SemaModule {
    Mempool *mempool;
    Parser *parser;
    AstNode **nodes;
    Hir *hir;
    SemaProject *project;
    bool no_std;

    bool failed;

    SemaStd std;
    SemaScopeStack *ss;
    SemaDecl **local_decls_map;
    SemaTypeInfo *types;
    bool *stage_failures;
} SemaModule;

Slice sema_module_internal_slice();
void sema_module_err(SemaModule *module, Slice where, const char *fmt, ...);
void sema_module_internal_err(SemaModule *module, const char *fmt, ...);
void sema_module_link_project(SemaModule *module, SemaProject *project);
void sema_module_make_no_std(SemaModule *module);
Path sema_module_file_path(SemaModule *module);

SemaDecl *sema_module_resolve_req_decl(SemaModule *module, Slice name);
SemaDecl *sema_module_resolve_req_decl_from(SemaModule *module, SemaModule *from, Slice name);
SemaDecl *sema_module_resolve_req_decl_from_at(SemaModule *module, SemaModule *from, Slice at, Slice name);
SemaScopeStack *sema_module_swap_ss(SemaModule *module, SemaScopeStack *new_ss);

void sema_module_add_defer(SemaModule *module, HirCode *code);
void sema_module_scope_break(SemaModule *module);
bool sema_module_scope_breaks(SemaModule *module);

void sema_module_emit_defers(SemaModule *module);
void sema_module_emit_current_defers(SemaModule *module);
void sema_module_emit_defers_before_loop(SemaModule *module, HirLoopId id);

void sema_module_push_scope_looped(SemaModule *module, SemaLoop *loop);
void sema_module_push_scope(SemaModule *module, SemaLoop *loop);
void sema_module_pop_scope(SemaModule *module);

SemaTypeId sema_module_register_type_alias(SemaModule *module);
void sema_module_init_type_alias(SemaModule *module, SemaTypeId id, SemaType *type);
HirTypeId sema_module_get_type_id(SemaModule *module, SemaTypeId id);

void sema_module_push_decl(SemaModule *module, Slice name, SemaDecl *decl);

bool sema_module_is_global_scope(SemaModule *module);
