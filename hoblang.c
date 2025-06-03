#include "core/log.h"
#include "core/mempool.h"
#include "core/vec.h"
#include "ir/api/ir.h"
#include "ir/stages/type_tree.h"
#include "ir/stmt/code.h"
#include "ir/stmt/expr.h"
#include "ir/stmt/stmt.h"
#include "ir/type/type.h"
#include "print.h"
#include "llvm/module/api.h"

static Ir *build_ir() {
    Ir *ir = ir_new();
    Mempool *irm = ir_mempool(ir);

    // Types
    IrTypeId t_i32 = ir_add_simple_type(ir, ir_type_new_int(IR_TYPE_INT_32, true));
    IrTypeCrossReference *crs = ir_type_check_tree(ir);
    assert(vec_len(crs) == 0);

    // Decls
    IrDeclId d_main = ir_add_decl(ir);

    // Funcs
    IrFuncId f_main = ir_init_func(ir, d_main, ir_func_new_global(
        slice_from_cstr("main"),
        vec_new_in(irm, IrFuncArg),
        t_i32
    ));

    // Code
    IrExpr exit_code = ir_expr_new(vec_create_in(irm,
        ir_expr_step_new_int(t_i32, 123),
    ));
    ir_init_func_body(ir, f_main, ir_code_new(irm, vec_create_in(irm,
        ir_stmt_new_ret(ir_mempool(ir), exit_code)
    )));
    return ir;
}

int main() {
    setup_log();
    Ir *ir = build_ir();
    LlvmModule *llvm = llvm_module_new();
    llvm_module_emit(llvm, ir);
    llvm_module_write_ir(llvm, "test.ll");
    if (!llvm_module_write_obj(llvm, "test.o")) {
        logln("failed to write module");
        return 1;
    }

    llvm_module_free(llvm);
    ir_free(ir);
    return 0;
}
