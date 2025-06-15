#include "const.h"

SemaConst *sema_const_new_integer(Mempool *mempool, SemaType *type, uint64_t integer)
    MEMPOOL_CONSTRUCT(SemaConst, 
        out->kind = SEMA_CONST_INT;
        out->type = type;
        out->integer = integer;
    )
