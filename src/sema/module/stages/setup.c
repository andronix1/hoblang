#include "setup.h"
#include "core/slice.h"
#include "sema/module/decl.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

static inline void sema_module_push_type(SemaModule *module, const char *name, SemaType *type) {
    sema_module_push_decl(module, slice_from_cstr(name), sema_decl_new(module->mempool,
        sema_value_new_type(module->mempool, type)));
}

void sema_module_setup(SemaModule *module) {
    sema_module_push_type(module, "bool", sema_type_new_bool(module));
    sema_module_push_type(module, "void", sema_type_new_void(module));
    sema_module_push_type(module, "u8", sema_type_new_int(module, SEMA_INT_8, false));
    sema_module_push_type(module, "i8", sema_type_new_int(module, SEMA_INT_8, true));
    sema_module_push_type(module, "u16", sema_type_new_int(module, SEMA_INT_16, false));
    sema_module_push_type(module, "i16", sema_type_new_int(module, SEMA_INT_16, true));
    sema_module_push_type(module, "u32", sema_type_new_int(module, SEMA_INT_32, false));
    sema_module_push_type(module, "i32", sema_type_new_int(module, SEMA_INT_32, true));
    sema_module_push_type(module, "u64", sema_type_new_int(module, SEMA_INT_64, false));
    sema_module_push_type(module, "i64", sema_type_new_int(module, SEMA_INT_64, true));
}
