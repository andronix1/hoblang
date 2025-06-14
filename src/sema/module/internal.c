#include "internal.h"
#include "core/null.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"
#include "sema/module/decl.h"
#include "sema/module/module.h"
#include "sema/module/type.h"

static SemaType *sema_module_internal_type(SemaModule *module, SemaModule *internal, Slice at, Slice name) {
    SemaType *type = sema_value_is_type(NOT_NULL(sema_module_resolve_req_decl_from_at(internal, module, at, name))->value);
    if (!type) {
        sema_module_err(module, at, "`$S` in `internal` module is not a type", name);
        return NULL;
    }
    return type;
}

static IrDeclId *sema_module_internal_value(SemaModule *module, SemaModule *internal, Slice at, Slice name, SemaType *type) {
    SemaDecl *decl = NOT_NULL(sema_module_resolve_req_decl_from_at(internal, module, at, name));
    SemaValueRuntime *runtime = sema_value_is_runtime(decl->value);
    if (!runtime) {
        sema_module_err(module, at, "`$S` in `internal` module is not a runtime value", type);
        return NULL;
    }

    if (!sema_type_eq(runtime->type, type)) {
        sema_module_err(module, at, "`$S` in `internal` module is not matching type $t", name, type);
        return NULL;
    }

    assert(runtime->val_kind == SEMA_VALUE_RUNTIME_GLOBAL);
    return &runtime->global_id;
}


IrDeclId *sema_module_internal_string_new(SemaModule *module, Slice at) {
    if (!module->internal.loaded) {
        SemaModule *internal_module = sema_project_add_library(module->project, slice_from_cstr("internal"));
        if (!internal_module) {
            sema_module_err(module, at, "cannot find `internal` module which is required for strings");
            return NULL;
        }

        SemaType *usize = sema_type_new_int(module, SEMA_INT_64, false);

        SemaType *str = sema_module_internal_type(module, internal_module, at, slice_from_cstr("Str"));
        IrDeclId id = *NOT_NULL(sema_module_internal_value(module, internal_module, at, slice_from_cstr("_newStr"),
            sema_type_new_function(module, vec_create_in(module->mempool, 
                sema_type_new_pointer(module, sema_type_new_int(module, SEMA_INT_8, false)),
                sema_type_new_int(module, SEMA_INT_64, false)
            ), str)));


        module->internal.loaded = true;
        module->internal.usize = usize;
        module->internal.string.type = str;
        module->internal.string.new = id;
    }
    return &module->internal.string.new;
}

