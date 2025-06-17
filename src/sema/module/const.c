#include "const.h"

SemaConst *sema_const_new_float(Mempool *mempool, SemaType *type, long double value)
    MEMPOOL_CONSTRUCT(SemaConst, 
        out->kind = SEMA_CONST_FLOAT;
        out->type = type;
        out->float_value = value;
    )
    
SemaConst *sema_const_new_integer(Mempool *mempool, SemaType *type, uint64_t integer)
    MEMPOOL_CONSTRUCT(SemaConst, 
        out->kind = SEMA_CONST_INT;
        out->type = type;
        out->integer = integer;
    )

SemaConst *sema_const_new_func(Mempool *mempool, SemaType *type, IrDeclId decl)
    MEMPOOL_CONSTRUCT(SemaConst, 
        out->kind = SEMA_CONST_FUNC;
        out->type = type;
        out->func_decl = decl;
    )
