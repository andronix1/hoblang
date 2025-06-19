#include "type.h"
#include "core/null.h"
#include "sema/module/ast/type.h"
#include "sema/module/decl.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

bool sema_module_stage_init_type_decl(SemaModule *module, AstTypeDecl *type_decl) {
    SemaTypeId id = sema_module_register_type_alias(module);
    type_decl->sema.type_id = id;
    HirTypeId hir_id = module->types[id].id;
    SemaType *type = sema_type_new_alias(module->mempool,
        sema_type_new_record(module, id), sema_type_alias_new(module->mempool, hir_id));
    type_decl->sema.type = type;
    sema_module_push_decl(module, type_decl->name, sema_decl_new(module->mempool,
        type_decl->is_public ? NULL : module, sema_value_new_type(module->mempool, type)));
    return true;
}

bool sema_module_stage_fill_type_decl(SemaModule *module, AstTypeDecl *type_decl) {
    SemaType *source_type = NOT_NULL(sema_module_type(module, type_decl->type));
    SemaTypeInfo *type_info = &module->types[type_decl->sema.type_id];
    type_info->type = source_type;
    hir_init_type(module->hir, type_info->id, sema_type_hir_id(source_type));
    return true;
}
