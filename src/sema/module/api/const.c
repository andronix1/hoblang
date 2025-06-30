#include "const.h"
#include "core/assert.h"
#include "sema/module/api/type.h"
#include "sema/module/const.h"
#include "sema/module/module.h"
#include <assert.h>
#include <string.h>

HirConst sema_const_to_hir(SemaModule *module, SemaConst *constant) {
    HirType *type = sema_type_to_hir(module, constant->type);
    switch (constant->kind) {
        case SEMA_CONST_INT: return hir_const_new_int(type, constant->integer);
        case SEMA_CONST_BOOL: return hir_const_new_bool(type, constant->boolean);
        case SEMA_CONST_FLOAT: return hir_const_new_real(type, constant->float_value);
        case SEMA_CONST_FUNC: return hir_const_new_func(type, constant->func_decl);
        case SEMA_CONST_UNDEFINED: return hir_const_new_undefined(type);
        case SEMA_CONST_STRUCT: {
            HirConst *fields = vec_new_in(module->mempool, HirConst);
            vec_resize(fields, vec_len(constant->struct_fields));
            for (size_t i = 0; i < vec_len(constant->struct_fields); i++) {
                fields[i] = sema_const_to_hir(module, constant->struct_fields[i]);
            }
            return hir_const_new_struct(type, fields);
        }
        case SEMA_CONST_GEN_FUNC: {
            HirType **params = vec_new_in(module->mempool, HirType*);
            vec_resize(params, vec_len(constant->gen_func.params));
            for (size_t i = 0; i < vec_len(constant->gen_func.params); i++) {
                params[i] = sema_type_to_hir(module, constant->gen_func.params[i]);
            }
            return vec_len(module->gen_scopes) ?
                hir_const_new_gen_func_from(type, constant->gen_func.scope, constant->gen_func.func, params,
                    vec_top(module->gen_scopes)->scope) :
                hir_const_new_gen_func(type, constant->gen_func.scope, constant->gen_func.func, params);
        }
    }
    UNREACHABLE;
}

SemaConst *sema_const_nest(Mempool *mempool, SemaConst *constant, SemaType *type) {
    SemaConst *result = mempool_alloc(mempool, SemaConst);
    memcpy(result, constant, sizeof(SemaConst));
    assert(sema_type_can_be_casted(type, result->type));
    result->type = type;
    return result;
}
