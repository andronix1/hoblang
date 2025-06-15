#include "const.h"
#include "core/assert.h"
#include "sema/module/const.h"
#include "ir/const.h"

IrConst *sema_const_to_ir(Mempool *mempool, SemaConst *constant) {
    switch (constant->kind) {
        case SEMA_CONST_INT: return ir_const_new_int(mempool, sema_type_ir_id(constant->type), constant->integer);
    }
    UNREACHABLE;
}
