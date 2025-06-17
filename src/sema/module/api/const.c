#include "const.h"
#include "core/assert.h"
#include "sema/module/api/type.h"
#include "sema/module/const.h"
#include "ir/const.h"
#include <assert.h>
#include <string.h>

IrConst *sema_const_to_ir(Mempool *mempool, SemaConst *constant) {
    switch (constant->kind) {
        case SEMA_CONST_INT: return ir_const_new_int(mempool, sema_type_ir_id(constant->type), constant->integer);
        case SEMA_CONST_FLOAT: return ir_const_new_real(mempool, sema_type_ir_id(constant->type), constant->float_value);
        case SEMA_CONST_FUNC: return ir_const_new_func(mempool, sema_type_ir_id(constant->type), constant->func_decl);
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
