#include "generic.h"
#include "core/mempool.h"
#include "ast/type.h"
#include "core/slice.h"

AstGeneric *ast_generics_new(Mempool *mempool, AstGenericParam *params)
    MEMPOOL_CONSTRUCT(AstGeneric, out->params = params;)

AstGenericBuilder *ast_generic_builder_new(Mempool *mempool, AstType **params)
    MEMPOOL_CONSTRUCT(AstGenericBuilder, out->params = params;)


Slice ast_generic_builder_slice(AstGenericBuilder *builder) {
    assert(vec_len(builder->params) > 0);
    Slice slice = builder->params[0]->slice;
    for (size_t i = 1; i < vec_len(builder->params); i++) {
        slice = slice_union(slice, builder->params[i]->slice);
    }
    return slice;
}
