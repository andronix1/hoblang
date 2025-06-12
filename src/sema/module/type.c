#include "type.h"
#include "core/assert.h"
#include "core/mempool.h"
#include "core/vec.h"
#include "ir/api/ir.h"
#include "ir/api/type.h"
#include "ir/type/type.h"
#include "sema/module/api/type.h"
#include "sema/module/module.h"
#include "sema/module/api/module.h"
#include <string.h>

IrTypeIntSize sema_type_int_size_to_ir(SemaTypeIntSize size) {
    switch (size) {
        case SEMA_INT_8: return IR_TYPE_INT_8;
        case SEMA_INT_16: return IR_TYPE_INT_16;
        case SEMA_INT_32: return IR_TYPE_INT_32;
        case SEMA_INT_64: return IR_TYPE_INT_64;
    }
    UNREACHABLE;
}

static inline IrType sema_type_to_ir(SemaModule* module, SemaType *type) {
    switch (type->kind) {
        case SEMA_TYPE_VOID: return ir_type_new_void();
        case SEMA_TYPE_INT:
            return ir_type_new_int(sema_type_int_size_to_ir(type->integer.size),
                type->integer.is_signed);
        case SEMA_TYPE_BOOL: return ir_type_new_bool();
        case SEMA_TYPE_FUNCTION: {
            IrTypeId *args = vec_new_in(module->mempool, IrTypeId);
            vec_resize(args, vec_len(type->function.args));
            for (size_t i = 0; i < vec_len(type->function.args); i++) {
                args[i] = sema_type_ir_id(type->function.args[i]);
            }
            return ir_type_new_function(args, sema_type_ir_id(type->function.returns));
        }
        case SEMA_TYPE_RECORD: TODO;
        case SEMA_TYPE_POINTER: return ir_type_new_pointer(sema_type_ir_id(type->pointer_to));
    }
    UNREACHABLE;
}

SemaTypeAlias *sema_type_alias_new(Mempool *mempool, IrTypeId id)
    MEMPOOL_CONSTRUCT(SemaTypeAlias,
        out->id = id;
        out->decls_map = keymap_new_in(mempool, SemaDecl*);
    )

#define SEMA_TYPE_CONSTRUCT(KIND, FIELDS) { \
        SemaType *out = mempool_alloc(module->mempool, SemaType); \
        out->kind = KIND; \
        FIELDS; \
        out->aliases = NULL; \
        out->ir_id = KIND == SEMA_TYPE_RECORD ? \
            sema_module_get_type_id(module, out->record.id) : \
            ir_add_simple_type(module->ir, sema_type_to_ir(module, out)); \
        return out; \
    }

SemaType *sema_type_new_void(SemaModule *module)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_VOID,)

SemaType *sema_type_new_record(SemaModule *module, size_t type_id)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_RECORD,
        out->record.id = type_id;
        out->record.module = module;
    )

SemaType *sema_type_new_bool(SemaModule *module)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_BOOL,)

SemaType *sema_type_new_int(SemaModule *module, SemaTypeIntSize size, bool is_signed)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_INT,
        out->integer.size = size;
        out->integer.is_signed = is_signed;
    )

SemaType *sema_type_new_pointer(SemaModule *module, SemaType *pointer_to)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_POINTER, out->pointer_to = pointer_to)

SemaType *sema_type_new_function(SemaModule *module, SemaType **args, SemaType *returns)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_FUNCTION,
        out->function.args = args;
        out->function.returns = returns;
    )

SemaType *sema_type_new_alias(Mempool *mempool, SemaType *type, SemaTypeAlias *alias) {
    SemaType *result = mempool_alloc(mempool, SemaType);
    memcpy(result, type, sizeof(SemaType));
    result->aliases = vec_new_in(mempool, SemaTypeAlias*);
    if (type->aliases) {
        vec_extend(result->aliases, type->aliases);
    }
    vec_push(result->aliases, alias);
    return result;
}

