#include "string.h"
#include "core/null.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"
#include "sema/module/decl.h"
#include "sema/module/module.h"
#include "sema/module/type.h"

IrDeclId *sema_module_internal_string_new(SemaModule *module, Slice at) {
    if (!module->internal.loaded) {
        SemaModule *internal_module = sema_project_add_library(module->project, slice_from_cstr("internal"));
        if (!internal_module) {
            sema_module_err(module, at, "cannot find `internal` module which is required for strings");
            return NULL;
        }

        SemaType *usize = sema_type_new_int(module, SEMA_INT_64, false);

        SemaDecl *str_slice = NOT_NULL(sema_module_resolve_req_decl_from_at(internal_module, module, at,
            slice_from_cstr("Str")));
        SemaType *str_slice_type = sema_value_is_type(str_slice->value);
        if (!str_slice_type) {
            sema_module_err(module, at, "`Str` in `internal` module is not a type");
            return NULL;
        }

        SemaDecl *decl = NOT_NULL(sema_module_resolve_req_decl_from_at(internal_module, module, at,
            slice_from_cstr("_newStr")));
        SemaValueRuntime *runtime = sema_value_is_runtime(decl->value);
        if (!runtime) {
            sema_module_err(module, at, "`_newStr` in `internal` module is not a runtime value");
            return NULL;
        }

        if (!sema_type_eq(runtime->type, sema_type_new_function(module, vec_create_in(module->mempool, 
            sema_type_new_pointer(module, sema_type_new_int(module, SEMA_INT_8, false)),
            usize, // TODO: usize type
        ), str_slice_type))) {
            sema_module_err(module, at,
                "`_newStr` in `internal` module is not matching a type fun (*u8, u64) -> Str");
            return NULL;
        }

        assert(runtime->val_kind == SEMA_VALUE_RUNTIME_GLOBAL);
        module->internal.loaded = true;
        module->internal.type = str_slice_type;
        module->internal.usize = usize;
        module->internal.new = runtime->global_id;
    }
    return &module->internal.new;
}

