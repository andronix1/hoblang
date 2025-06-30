#include "std.h"
#include "core/keymap.h"
#include "core/mempool.h"
#include "core/null.h"
#include "core/vec.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"
#include "sema/module/const.h"
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

/*
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
*/

static bool sema_type_struct_matches(SemaType *type, SemaType **fields) {
    type = sema_type_root(type);
    if (type->kind != SEMA_TYPE_STRUCTURE) {
        return false;
    }
    if (vec_len(type->structure.fields_map) != vec_len(fields)) {
        return false;
    }
    for (size_t i = 0; i < vec_len(type->structure.fields_map); i++) {
        keymap_at(type->structure.fields_map, i, field);
        if (!sema_type_eq(field->value.type, fields[i])) {
            return false;
        }
    }
    return true;
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
    SemaType *str = NOT_NULL(sema_module_internal_type(module, internal_module, at, slice_from_cstr("string")));
    if (!sema_type_struct_matches(str, vec_create_in(module->mempool, 
        sema_type_new_pointer(module->mempool, sema_type_new_int(module->mempool, SEMA_INT_8, false)),
        usize,
    ))) {
        sema_module_err(module, at, "string's structure doesn't matches { *u8, usize }");
        return false;
    }


    module->std.usize = usize;
    module->std.string.type = str;
    return true;
}

SemaConst *sema_module_std_new_string(SemaModule *module, Slice at, SemaConst *pointer, SemaConst *len) {
    NOT_NULL(sema_module_std_load(module, at));
    return sema_const_new_struct(module->mempool, module->std.string.type, vec_create_in(module->mempool, pointer, len));
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

SemaType *sema_module_std_usize(SemaModule *module, Slice at) {
    NOT_NULL(sema_module_std_load(module, at));
    return module->std.usize;
}

