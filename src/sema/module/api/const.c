#include "const.h"
#include "core/assert.h"
#include "sema/module/api/type.h"
#include "sema/module/const.h"
#include "sema/module/module.h"
#include <assert.h>
#include <string.h>

HirConst sema_const_to_hir(SemaModule *module, SemaConst *constant) {
    switch (constant->kind) {
        case SEMA_CONST_INT: return hir_const_new_int(sema_type_hir_id(constant->type), constant->integer);
        case SEMA_CONST_FLOAT: return hir_const_new_real(sema_type_hir_id(constant->type), constant->float_value);
        case SEMA_CONST_FUNC: return hir_const_new_func(sema_type_hir_id(constant->type), constant->func_decl);
        case SEMA_CONST_UNDEFINED: return hir_const_new_undefined(sema_type_hir_id(constant->type));
        case SEMA_CONST_GEN_FUNC: {
            HirTypeId type_id = sema_type_hir_id(constant->type);
            return vec_len(module->gen_scopes) ?
                hir_const_new_gen_func_from(type_id, constant->gen_func.scope, constant->gen_func.func,
                    constant->gen_func.params, *vec_top(module->gen_scopes)) :
                hir_const_new_gen_func(type_id, constant->gen_func.scope, constant->gen_func.func,
                    constant->gen_func.params);
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
