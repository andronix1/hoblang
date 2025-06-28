#include "setup.h"
#include "core/slice.h"
#include "sema/module/api/module.h"
#include "sema/module/decl.h"
#include "sema/module/module.h"
#include "sema/module/stages/stages.h"
#include "sema/module/type.h"
#include "sema/module/value.h"
#include <stdio.h>

static inline void sema_module_push_type(SemaModule *module, const char *name, SemaType *type) {
    sema_module_push_decl(module, slice_from_cstr(name), sema_decl_new(module->mempool, module,
        sema_value_new_type(module->mempool, type)));
}

void sema_module_setup(SemaModule *module) {
    sema_module_push_type(module, "bool", sema_type_new_bool(module->mempool));
    sema_module_push_type(module, "void", sema_type_new_void(module->mempool));
    sema_module_push_type(module, "u8", sema_type_new_int(module->mempool, SEMA_INT_8, false));
    sema_module_push_type(module, "i8", sema_type_new_int(module->mempool, SEMA_INT_8, true));
    sema_module_push_type(module, "u16", sema_type_new_int(module->mempool, SEMA_INT_16, false));
    sema_module_push_type(module, "i16", sema_type_new_int(module->mempool, SEMA_INT_16, true));
    sema_module_push_type(module, "u32", sema_type_new_int(module->mempool, SEMA_INT_32, false));
    sema_module_push_type(module, "i32", sema_type_new_int(module->mempool, SEMA_INT_32, true));
    sema_module_push_type(module, "u64", sema_type_new_int(module->mempool, SEMA_INT_64, false));
    sema_module_push_type(module, "i64", sema_type_new_int(module->mempool, SEMA_INT_64, true));
    sema_module_push_type(module, "f32", sema_type_new_float(module->mempool, SEMA_FLOAT_32));
    sema_module_push_type(module, "f64", sema_type_new_float(module->mempool, SEMA_FLOAT_64));

    if (!module->no_std) {
        sema_module_push_type(module, "string", sema_module_std_string(module, sema_module_internal_slice()));
        sema_module_push_type(module, "usize", sema_module_std_usize(module, sema_module_internal_slice()));
    }
    
    for (size_t i = 0; i < sema_setup_stages_count; i++) {
        sema_module_run_stage(module, i);
    }
}
