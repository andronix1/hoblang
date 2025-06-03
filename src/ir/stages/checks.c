#include "checks.h"
#include "ir/ir.h"

static void ir_check_types(const Ir *ir) {
    for (size_t i = 0; i < vec_len(ir->types); i++) {
        IrTypeInfo *info = &ir->types[i];
        if (info->kind == IR_TYPE_INFO_RECORD) {
            assert(info->record.filled);
        }
    }
}

static void ir_check_funcs(const Ir *ir) {
    for (size_t i = 0; i < vec_len(ir->funcs); i++) {
        assert(ir->funcs[i].code);
    }
}

static void ir_check_decls(const Ir *ir) {
    for (size_t i = 0; i < vec_len(ir->decls); i++) {
        assert(ir->decls[i].filled);
    }
}

void ir_check_cosistency(const Ir *ir) {
    ir_check_types(ir);
    ir_check_decls(ir);
    ir_check_funcs(ir);
}
