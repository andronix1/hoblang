#pragma once

#include "ir/api/const.h"
#include "ir/api/decl.h"
#include "ir/api/extern.h"
#include "ir/api/func.h"
#include "ir/api/type.h"
#include "ir/api/var.h"
#include "ir/extern.h"
#include "ir/func.h"
#include "ir/stmt/stmt.h"
#include "ir/type/type.h"
#include "ir/var.h"

typedef struct Ir Ir;

Ir *ir_new();

IrTypeId ir_add_simple_type(Ir *ir, IrType type);
IrTypeId ir_add_type_record(Ir *ir);
void ir_set_type_record(Ir *ir, IrTypeId id, IrTypeId type);

IrDeclId ir_add_decl(Ir *ir);

IrExternId ir_init_extern(Ir *ir, IrDeclId id, IrExtern ext);

IrVarId ir_init_var(Ir *ir, IrDeclId id, IrVar var);
void ir_set_var_initializer(Ir *ir, IrVarId id, IrConst *initializer);

IrLocalId ir_func_arg_local_id(Ir *ir, IrFuncId id, size_t arg_id);
IrFuncId ir_init_func(Ir *ir, IrMutability *args_mut, IrDeclId id, IrFunc func);
IrLocalId ir_func_add_local(Ir *ir, IrFuncId id, IrFuncLocal local);
IrLoopId ir_func_add_loop(Ir *ir, IrFuncId id);
void ir_init_func_body(Ir *ir, IrFuncId id, IrCode *code);

bool ir_type_int_is_signed(Ir *ir, IrTypeId id);
IrDeclId ir_func_decl_id(Ir *ir, IrFuncId id);

Mempool *ir_mempool(Ir *ir);

void ir_free(Ir *ir);
