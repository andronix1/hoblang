#pragma once

#include "core/mempool.h"
#include "ir/api/decl.h"
#include "ir/api/const.h"
#include "ir/api/type.h"
#include <stdint.h>

typedef enum {
    IR_CONST_BOOL,
    IR_CONST_INT,
    IR_CONST_REAL,
    IR_CONST_STRUCT,
    IR_CONST_DECL_PTR,
} IrConstKind;

typedef struct IrConst {
    IrConstKind kind;

    IrTypeId type;

    union {
        bool boolean;
        uint64_t integer;
        long double real;
        IrConst **struct_fields;
        IrDeclId decl;
    };
} IrConst;

IrConst *ir_const_new_int(Mempool *mempool, IrTypeId type, uint64_t value);
IrConst *ir_const_new_bool(Mempool *mempool, IrTypeId type, bool value);
IrConst *ir_const_new_real(Mempool *mempool, IrTypeId type, long double value);
IrConst *ir_const_new_struct(Mempool *mempool, IrTypeId type, IrConst **fields);
IrConst *ir_const_new_decl(Mempool *mempool, IrTypeId type, IrDeclId decl);
