#include "generic.h"
#include "core/assert.h"
#include "core/mempool.h"
#include "core/vec.h"
#include "sema/module/api/type.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

SemaValue *sema_generate(SemaGeneric *generic, SemaType **input) {
    assert(vec_len(input) == vec_len(generic->params));
    switch (generic->kind) {
        case SEMA_GENERIC_TYPE:
            return sema_value_new_type(generic->module->mempool, sema_type_new_generate(
                    generic->module, generic, input));
    }
    UNREACHABLE;
}

SemaGeneric *sema_generic_new(Mempool *mempool, SemaGenericKind kind, SemaModule *module, SemaType **params)
    MEMPOOL_CONSTRUCT(SemaGeneric,
        out->kind = kind;
        out->params = params;
        out->module = module;
    )
