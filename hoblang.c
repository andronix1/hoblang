#include "core/log.h"
#include "core/mempool.h"
#include "core/slice.h"
#include "core/vec.h"
#include "ir/api/ir.h"
#include "ir/func.h"
#include "ir/stages/stmts.h"
#include "ir/stages/type_tree.h"
#include "ir/stmt/code.h"
#include "ir/stmt/expr.h"
#include "ir/stmt/stmt.h"
#include "ir/type/type.h"
#include "print.h"
#include "llvm/module/api.h"

IrExpr call_put_char(Mempool *mempool, IrTypeId t_u8, IrDeclId d_put_char, char c) {
    size_t *args = vec_create_in(mempool, (size_t)0);
    return ir_expr_new(vec_create_in(mempool,
        ir_expr_step_new_int(t_u8, c),
        ir_expr_step_new_get_decl(d_put_char),
        ir_expr_step_new_call(args, 1)
    ));
}

static Ir *build_ir() {
    Ir *ir = ir_new();
    Mempool *irm = ir_mempool(ir);

    // Types
    IrTypeId t_void = ir_add_simple_type(ir, ir_type_new_void());
    IrTypeId t_i32 = ir_add_simple_type(ir, ir_type_new_int(IR_TYPE_INT_32, true));
    IrTypeId t_u8 = ir_add_simple_type(ir, ir_type_new_int(IR_TYPE_INT_8, false));
    IrTypeId t_put_char_func = ir_add_simple_type(ir, ir_type_new_function(
        vec_create_in(irm, t_u8),
        t_void
    ));
    IrTypeCrossReference *crs = ir_type_check_tree(ir);
    assert(vec_len(crs) == 0);

    // Decls
    IrDeclId d_main = ir_add_decl(ir);
    IrDeclId d_put_char = ir_add_decl(ir);

    // Funcs
    IrFuncId f_main = ir_init_func(ir, d_main, ir_func_new_global(
        slice_from_cstr("main"),
        vec_new_in(irm, IrFuncArg),
        t_i32
    ));
    ir_init_extern(ir, d_put_char, ir_extern_new(
        IR_EXTERN_FUNC,
        slice_from_cstr("putchar"),
        t_put_char_func
    ));

    // Code
    IrLocalId l_exit_code = ir_func_add_local(ir, d_main,
        ir_func_local_new(IR_MUTABLE, t_i32));
    IrExpr exit_code_set = ir_expr_new(vec_create_in(irm,
        ir_expr_step_new_int(t_i32, 123),
    ));

    IrExpr exit_code = ir_expr_new(vec_create_in(irm,
        ir_expr_step_new_get_local(l_exit_code),
    ));


    ir_init_func_body(ir, f_main, ir_code_new(irm, vec_create_in(irm,
        ir_stmt_new_expr(irm, call_put_char(irm, t_u8, d_put_char, 'h')),
        ir_stmt_new_expr(irm, call_put_char(irm, t_u8, d_put_char, 'e')),
        ir_stmt_new_expr(irm, call_put_char(irm, t_u8, d_put_char, 'l')),
        ir_stmt_new_expr(irm, call_put_char(irm, t_u8, d_put_char, 'l')),
        ir_stmt_new_expr(irm, call_put_char(irm, t_u8, d_put_char, 'o')),
        ir_stmt_new_expr(irm, call_put_char(irm, t_u8, d_put_char, '!')),
        ir_stmt_new_expr(irm, call_put_char(irm, t_u8, d_put_char, '\n')),
        ir_stmt_new_decl_var(irm, l_exit_code),
        ir_stmt_new_store(irm, exit_code, exit_code_set),
        ir_stmt_new_ret(irm, exit_code)
    )));

    ir_fill_stmts(ir);
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
