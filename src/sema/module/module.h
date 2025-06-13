#pragma once

#include "ast/api/node.h"
#include "core/mempool.h"
#include "core/slice.h"
#include "ir/api/ir.h"
#include "parser/api.h"
#include "sema/api/project.h"
#include "sema/module/api/decl.h"
#include "sema/module/api/type.h"
#include "sema/module/api/decl.h"
#include "sema/module/scope.h"

typedef struct SemaTypeInfo {
    SemaType *type;
    IrTypeId id;
} SemaTypeInfo;

typedef struct SemaModule {
    Mempool *mempool;
    Parser *parser;
    AstNode **nodes;
    Ir *ir;
    SemaProject *project;

    bool failed;

    SemaScopeStack *ss;
    SemaDecl **local_decls_map;
    SemaTypeInfo *types;
    bool *stage_failures;
} SemaModule;

void sema_module_err(SemaModule *module, Slice where, const char *fmt, ...);
void sema_module_link_project(SemaModule *module, SemaProject *project);
Path sema_module_file_path(SemaModule *module);

void sema_module_push_loop(SemaModule *module, SemaLoop loop);
void sema_module_pop_loop(SemaModule *module);
SemaDecl *sema_module_resolve_req_decl(SemaModule *module, Slice name);
SemaScopeStack *sema_module_swap_ss(SemaModule *module, SemaScopeStack *new_ss);
void sema_module_push_scope(SemaModule *module);
void sema_module_pop_scope(SemaModule *module);

SemaTypeId sema_module_register_type_alias(SemaModule *module);
void sema_module_init_type_alias(SemaModule *module, SemaTypeId id, SemaType *type);
IrTypeId sema_module_get_type_id(SemaModule *module, SemaTypeId id);

void sema_module_push_decl(SemaModule *module, Slice name, SemaDecl *decl);

bool sema_module_is_global_scope(SemaModule *module);
