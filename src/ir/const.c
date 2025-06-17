#include "const.h"

IrConst *ir_const_new_int(Mempool *mempool, IrTypeId type, uint64_t value)
    MEMPOOL_CONSTRUCT(IrConst,
        out->kind = IR_CONST_INT;
        out->type = type;
        out->integer = value;
    )

IrConst *ir_const_new_bool(Mempool *mempool, IrTypeId type, bool value)
    MEMPOOL_CONSTRUCT(IrConst,
        out->kind = IR_CONST_BOOL;
        out->type = type;
        out->boolean = value;
    )

IrConst *ir_const_new_real(Mempool *mempool, IrTypeId type, long double value)
    MEMPOOL_CONSTRUCT(IrConst,
        out->kind = IR_CONST_REAL;
        out->type = type;
        out->real = value;
    )

IrConst *ir_const_new_struct(Mempool *mempool, IrTypeId type, IrConst **fields)
    MEMPOOL_CONSTRUCT(IrConst,
        out->kind = IR_CONST_STRUCT;
        out->type = type;
        out->struct_fields = fields;
    )

IrConst *ir_const_new_func(Mempool *mempool, IrTypeId type, IrDeclId decl)
    MEMPOOL_CONSTRUCT(IrConst,
        out->kind = IR_CONST_FUNC;
        out->type = type;
        out->func_decl = decl;
    )
