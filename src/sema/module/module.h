#pragma once

#include "ast/api/node.h"
#include "core/mempool.h"
#include "core/slice.h"
#include "ir/api/ir.h"
#include "parser/api.h"
#include "sema/module/api/decl.h"
#include "sema/module/api/type.h"
#include "sema/module/api/decl.h"

typedef struct SemaTypeInfo {
    SemaType *type;
    IrTypeId id;
} SemaTypeInfo;


typedef struct SemaModule {
    Mempool *mempool;
    Parser *parser;
    AstNode **nodes;
    Ir *ir;
    bool failed;

    SemaDecl **local_decls_map;

    SemaTypeInfo *types;
} SemaModule;

void sema_module_err(SemaModule *module, Slice where, const char *fmt, ...);
SemaDecl *sema_module_resolve_req_decl(SemaModule *module, Slice name);

SemaTypeId sema_module_register_type_alias(SemaModule *module);
void sema_module_init_type_alias(SemaModule *module, SemaTypeId id, SemaType *type);
IrTypeId sema_module_get_type_id(SemaModule *module, SemaTypeId id);

void sema_module_push_decl(SemaModule *module, Slice name, SemaDecl *decl);

bool sema_module_is_global_scope(SemaModule *module);
