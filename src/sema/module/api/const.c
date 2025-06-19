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
    }
    UNREACHABLE;
}

SemaConst *sema_const_nest(Mempool *mempool, SemaConst *constant, SemaType *type) {
    SemaConst *result = mempool_alloc(mempool, SemaConst);
    memcpy(result, constant, sizeof(SemaConst));
    assert(sema_type_eq(type, result->type));
    result->type = type;
    return result;
}
