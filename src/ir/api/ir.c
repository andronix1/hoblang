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
    ir->vars = vec_new_in(mempool, IrVarInfo);
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

static IrFuncInfo ir_func_info_new(
    Ir *ir,
    IrMutability *args_mut,
    IrFunc func,
    IrDeclId id,
    IrTypeId type_id
) {
    IrType *type = &ir->types[ir_type_record_resolve_simple(ir, type_id)].simple;
    assert(type->kind == IR_TYPE_FUNCTION);
    assert(vec_len(type->function.args) == vec_len(args_mut));

    IrLocalId *args = vec_new_in(ir->mempool, IrLocalId);
    vec_resize(args, vec_len(type->function.args));
    IrFuncLocal *locals = vec_new_in(ir->mempool, IrFuncLocal);
    vec_resize(locals, vec_len(type->function.args));

    for (size_t i = 0; i < vec_len(type->function.args); i++) {
        locals[i] = ir_func_local_new(args_mut[i], type->function.args[i]);
        args[i] = i;
    }
    IrFuncInfo info = {
        .func = func,
        .args = args,
        .locals = locals,
        .decl_id = id,
        .type_id = type_id,
        .loops_count = 0,
    };
    return info;
}


IrFuncId ir_init_func(Ir *ir, IrMutability *args_mut, IrDeclId id, IrFunc func) {
    ir_init_decl(ir, id, IR_IMMUTABLE, func.type_id);
    vec_push(ir->funcs, ir_func_info_new(ir, args_mut, func, id, func.type_id));
    return vec_len(ir->funcs) - 1;
}

IrVarId ir_init_var(Ir *ir, IrDeclId id, IrVar var) {
    ir_init_decl(ir, id, IR_MUTABLE, var.type);
    vec_push(ir->vars, ir_var_info_new(var, id));
    return vec_len(ir->vars) - 1;
}

void ir_set_var_initializer(Ir *ir, IrVarId id, IrConst *initializer) {
    ir->vars[id].var.initializer = initializer;
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

IrLoopId ir_func_add_loop(Ir *ir, IrFuncId id) {
    return ir->funcs[id].loops_count++;
}

IrLocalId ir_func_arg_local_id(Ir *ir, IrFuncId id, size_t arg_id) {
    return ir->funcs[id].args[arg_id];
}

bool ir_type_int_is_signed(Ir *ir, IrTypeId id) {
    IrType *type = &ir->types[ir_type_record_resolve_simple(ir, id)].simple;
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
