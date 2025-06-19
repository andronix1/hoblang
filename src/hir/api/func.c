#include "func.h"

HirFuncInfo hir_func_info_new(Mempool *mempool, OptSlice global_name) {
    HirFuncInfo info = {
        .global_name = global_name,
        .code = NULL,
        .locals = vec_new_in(mempool, HirFuncLocal),
        .loops_count = 0
    };
    return info;
}
