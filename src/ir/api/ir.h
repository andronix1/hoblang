#pragma once

#include "ir/api/decl.h"
#include "ir/api/func.h"
#include "ir/api/type.h"
#include "ir/func.h"
#include "ir/type/type.h"

typedef struct Ir Ir;

Ir *ir_new();

IrTypeId ir_add_simple_type(Ir *ir, IrType type);
IrTypeId ir_add_type_record(Ir *ir);
void ir_set_type_record(Ir *ir, IrTypeId id, IrTypeId type);

IrDeclId ir_add_decl(Ir *ir);
IrLocalId ir_func_arg_local_id(Ir *ir, IrFuncId id, size_t arg_id);
IrFuncId ir_init_func(Ir *ir, IrDeclId id, IrFunc func);
IrLocalId ir_func_add_local(Ir *ir, IrFuncId id, IrFuncLocal local);
void ir_init_func_body(Ir *ir, IrFuncId id, IrCode *code);

void ir_free(Ir *ir);
