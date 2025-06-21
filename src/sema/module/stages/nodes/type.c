#include "type.h"
#include "core/null.h"
#include "sema/module/ast/generic.h"
#include "sema/module/ast/type.h"
#include "sema/module/decl.h"
#include "sema/module/generic.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "sema/module/value.h"
#include <stdio.h>

bool sema_module_stage_init_type_decl(SemaModule *module, AstTypeDecl *type_decl) {
    SemaModule *scope_module = type_decl->is_public ? NULL : module;
    if (type_decl->generics) {
        SemaGeneric *generic = NOT_NULL(sema_module_generic(module, SEMA_GENERIC_TYPE, type_decl->generics));
        sema_module_push_decl(module, type_decl->name, sema_decl_new(module->mempool, scope_module,
            sema_value_new_generic(module->mempool, generic)));
        type_decl->sema.generic = generic;
        return true;
    } else {
        SemaTypeId id = sema_module_register_type_alias(module);
        type_decl->sema.type_id = id;

        SemaType *type = sema_type_new_alias(module->mempool, sema_type_new_record(module, id),
            sema_type_alias_new(module->mempool, type_decl->name, module->types[id].id));
        sema_module_push_decl(module, type_decl->name, sema_decl_new(module->mempool,
            scope_module, sema_value_new_type(module->mempool, type)));
        type_decl->sema.type = type;
        return true;
    }
}

bool sema_module_stage_fill_type_generics(SemaModule *module, AstTypeDecl *type_decl) {
    if (type_decl->generics) {
        SemaGenericCtx ctx = sema_module_generic_ctx_setup(module, type_decl->generics, type_decl->sema.generic);
        SemaType *type = NOT_NULL(sema_module_type(module, type_decl->type));
        sema_module_generic_ctx_clean(module, ctx);
        sema_generic_fill_type(type_decl->sema.generic, type);
    }
    return true;
}

bool sema_module_stage_fill_type_decl(SemaModule *module, AstTypeDecl *type_decl) {
    if (!type_decl->generics){
        SemaType *source_type = NOT_NULL(sema_module_type(module, type_decl->type));
        SemaTypeInfo *type_info = &module->types[type_decl->sema.type_id];
        type_info->type = source_type;
        hir_init_type(module->hir, type_info->id, sema_type_hir_id(source_type));
    }
    return true;
}
