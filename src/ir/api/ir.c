#include "ir.h"
#include "core/assert.h"
#include "core/vec.h"
#include "ir/api/extern.h"
#include "ir/decls.h"
#include "ir/extern.h"
#include "ir/func.h"
#include "ir/ir.h"
#include "core/mempool.h"
#include "ir/type/type.h"
#include <assert.h>
#include <malloc.h>

Ir *ir_new() {
    Ir *ir = malloc(sizeof(Ir));
    Mempool *mempool = ir->mempool = mempool_new(1024);
    ir->types = vec_new_in(mempool, IrTypeInfo);
    ir->decls = vec_new_in(mempool, IrDecl);
    ir->funcs = vec_new_in(mempool, IrFuncInfo);
    ir->externs = vec_new_in(mempool, IrExternInfo);
    return ir;
}

IrTypeId ir_add_simple_type(Ir *ir, IrType type) {
    for (size_t i = 0; i < vec_len(ir->types); i++) {
        IrTypeInfo *info = &ir->types[i];
        if (info->kind != IR_TYPE_INFO_SIMPLE) {
            continue;
        }
        if (ir_type_eq(&info->simple, &type)) {
            return i;
        }
    }
    vec_push(ir->types, ir_type_info_new_simple(type));
    return vec_len(ir->types) - 1;
}

IrTypeId ir_add_type_record(Ir *ir) {
    vec_push(ir->types, ir_type_info_new_record());
    return vec_len(ir->types) - 1;
}

void ir_set_type_record(Ir *ir, IrTypeId id, IrTypeId type_id) {
    IrTypeInfo *info = &ir->types[id];
    assert(info->kind == IR_TYPE_INFO_RECORD);
    assert(!info->record.filled);
    info->record.filled = true;
    info->record.id = type_id;
}

IrDeclId ir_add_decl(Ir *ir) {
    vec_push(ir->decls, ir_decl_new());
    return vec_len(ir->decls) - 1;
}

static inline void ir_init_decl(Ir *ir, IrDeclId id, IrMutability mutability, IrTypeId type_id) {
    IrDecl *decl = &ir->decls[id];
    assert(!decl->filled);
    ir_decl_fill(decl, mutability, type_id);
}

static inline IrMutability ir_extern_get_mutability(IrExternKind kind) {
    switch (kind) {
        case IR_EXTERN_FUNC:
            return IR_IMMUTABLE;
        case IR_EXTERN_VAR:
            return IR_MUTABLE;
    }
    UNREACHABLE;
}

IrExternId ir_init_extern(Ir *ir, IrDeclId id, IrExtern ext) {
    ir_init_decl(ir, id, ir_extern_get_mutability(ext.kind), ext.type);
    vec_push(ir->externs, ir_extern_info_new(ext, id));
    return vec_len(ir->externs) - 1;
}

IrFuncId ir_init_func(Ir *ir, IrDeclId id, IrFunc func) {
    IrTypeId *args = vec_new_in(ir->mempool, IrTypeId);
    vec_resize(args, vec_len(func.args));
    for (size_t i = 0; i < vec_len(func.args); i++) {
        args[i] = func.args[i].type;
    }
    IrTypeId type_id = ir_add_simple_type(ir, ir_type_new_function(args, func.returns));
    ir_init_decl(ir, id, IR_IMMUTABLE, type_id);
    vec_push(ir->funcs, ir_func_info_new(ir->mempool, func, id, type_id));
    return vec_len(ir->funcs) - 1;
}

void ir_init_func_body(Ir *ir, IrFuncId id, IrCode *code) {
    IrFunc *func = &ir->funcs[id].func;
    assert(!func->code);
    func->code = code;
}

IrLocalId ir_func_add_local(Ir *ir, IrFuncId id, IrFuncLocal local) {
    IrFuncInfo *info = &ir->funcs[id];
    vec_push(info->locals, local);
    return vec_len(info->locals) - 1;
}

IrLocalId ir_func_arg_local_id(Ir *ir, IrFuncId id, size_t arg_id) {
    return ir->funcs[id].args[arg_id];
}

static inline IrType *ir_type_is_simple(Ir *ir, IrTypeId id) {
    IrTypeInfo *info = &ir->types[id];
    assert(info->kind == IR_TYPE_INFO_SIMPLE);
    return &info->simple;
}

bool ir_type_int_is_signed(Ir *ir, IrTypeId id) {
    IrType *type = ir_type_is_simple(ir, id);
    assert(type->kind == IR_TYPE_INT);
    return type->integer.is_signed;
}

IrDeclId ir_func_decl_id(Ir *ir, IrFuncId id) {
    return ir->funcs[id].decl_id;
}

Mempool *ir_mempool(Ir *ir) {
    return ir->mempool;
}

void ir_free(Ir *ir) {
    mempool_free(ir->mempool);
    free(ir);
}
