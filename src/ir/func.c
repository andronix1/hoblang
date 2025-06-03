#include "func.h"
#include "core/vec.h"

IrFuncInfo ir_func_info_new(Mempool *mempool, IrFunc func, IrDeclId id, IrTypeId type_id) {
    IrLocalId *args = vec_new_in(mempool, IrLocalId);
    vec_resize(args, vec_len(func.args));
    IrFuncLocal *locals = vec_new_in(mempool, IrFuncLocal);
    vec_resize(locals, vec_len(func.args));
    for (size_t i = 0; i < vec_len(func.args); i++) {
        locals[i] = ir_func_local_new(func.args[i].mutability, func.args[i].type);
        args[i] = i;
    }
    IrFuncInfo info = {
        .func = func,
        .args = args,
        .locals = locals,
        .decl_id = id,
        .type_id = type_id,
    };
    return info;
}

