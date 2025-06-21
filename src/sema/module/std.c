#include "std.h"
#include "core/null.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"
#include "sema/module/decl.h"
#include "sema/module/module.h"
#include "sema/module/type.h"

#define MODULE_NAME "std"

static SemaType *sema_module_internal_type(SemaModule *module, SemaModule *internal, Slice at, Slice name) {
    SemaType *type = sema_value_is_type(NOT_NULL(sema_module_resolve_req_decl_from_at(internal, internal, at, name))->value);
    if (!type) {
        sema_module_err(module, at, "`$S` in `"MODULE_NAME"` module is not a type", name);
        return NULL;
    }
    return type;
}

static SemaValueRuntime *sema_module_internal_value(SemaModule *module, SemaModule *internal, Slice at, Slice name, SemaType *type) {
    SemaDecl *decl = NOT_NULL(sema_module_resolve_req_decl_from_at(internal, internal, at, name));
    SemaValueRuntime *runtime = sema_value_is_runtime(decl->value);
    if (!runtime) {
        sema_module_err(module, at, "`$S` in `"MODULE_NAME"` module is not a runtime value", type);
        return NULL;
    }

    if (!sema_type_can_be_downcasted(runtime->type, type)) {
        sema_module_err(module, at, "`$S` in `"MODULE_NAME"` module is not matching type $t", name, type);
        return NULL;
    }

    return runtime;
}

static inline bool _sema_module_std_load(SemaModule *module, Slice at) {
    if (module->no_std) {
        sema_module_err(module, at, "feature is disabled in no-std mode");
    }

    SemaModule *internal_module = sema_project_add_internal_library(module->project, slice_from_cstr(MODULE_NAME));
    if (!internal_module) {
        sema_module_err(module, at, "cannot find `"MODULE_NAME"` module which is required for strings");
        return false;
    }

    SemaType *usize = NOT_NULL(sema_module_internal_type(module, internal_module, at, slice_from_cstr("usize")));
    SemaType *str = sema_module_internal_type(module, internal_module, at, slice_from_cstr("string"));
    SemaValueRuntime *newString = sema_module_internal_value(module, internal_module, at, slice_from_cstr("newString"),
        sema_type_new_function(module, vec_create_in(module->mempool, 
            sema_type_new_pointer(module, sema_type_new_int(module, SEMA_INT_8, false)),
            usize 
        ), str));

    module->std.usize = usize;
    module->std.string.type = str;
    module->std.string.new = newString;
    return true;
}

bool sema_module_std_load(SemaModule *module, Slice at) {
    switch (module->std.state) {
        case SEMA_STD_UNLOADED: break;
        case SEMA_STD_FAILED: return false;
        case SEMA_STD_LOADED: return true;
    }
    bool result = _sema_module_std_load(module, at);
    module->std.state = result ? SEMA_STD_LOADED : SEMA_STD_FAILED;
    return result;
}

SemaType *sema_module_std_string(SemaModule *module, Slice at) {
    NOT_NULL(sema_module_std_load(module, at));
    return module->std.string.type;
}

SemaValueRuntime *sema_module_std_string_new(SemaModule *module, Slice at) {
    NOT_NULL(sema_module_std_load(module, at));
    return module->std.string.new;
}

SemaType *sema_module_std_usize(SemaModule *module, Slice at) {
    NOT_NULL(sema_module_std_load(module, at));
    return module->std.usize;
}

