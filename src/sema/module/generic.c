#include "generic.h"
#include "core/mempool.h"
#include "sema/module/api.h"
#include "sema/module/module.h"

SemaGeneric *sema_generic_new(SemaModule *module, SemaType *type, SemaGenericParam *params) {
    Mempool *mempool = module->mempool;
    MEMPOOL_CONSTRUCT(SemaGeneric,
        out->kind = SEMA_GENERIC_TYPE;
        out->in = module;
        out->type = sema_generic_type_info_new(module->mempool, params, type);
        out->params = params;
    )
}
