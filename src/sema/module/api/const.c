#include "const.h"
#include "core/assert.h"
#include "sema/module/api/type.h"
#include "sema/module/const.h"
#include <assert.h>
#include <string.h>

HirConst sema_const_to_hir(SemaConst *constant) {
    switch (constant->kind) {
        case SEMA_CONST_INT: return hir_const_new_int(sema_type_hir_id(constant->type), constant->integer);
        case SEMA_CONST_FLOAT: return hir_const_new_real(sema_type_hir_id(constant->type), constant->float_value);
        case SEMA_CONST_FUNC: return hir_const_new_func(sema_type_hir_id(constant->type), constant->func_decl);
        case SEMA_CONST_GEN_FUNC: return hir_const_new_gen_func(sema_type_hir_id(constant->type), constant->gen_func.scope,
                constant->gen_func.func, constant->gen_func.params);
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
