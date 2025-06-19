#include "hir.test.h"
#include "core/mempool.h"
#include "hir/api/code.h"
#include "hir/api/const.h"
#include "hir/api/expr.h"
#include "hir/api/extern.h"
#include "hir/api/func.h"
#include "hir/api/hir.h"
#include "hir/api/mut.h"
#include "parser/test/common.test.h"
#include "llvm/module/api.h"
#include <CUnit/Basic.h>

static void test_hir_add_type() {
    Hir *hir = hir_new();
    HirTypeId t1 = hir_add_type(hir, hir_type_new_int(HIR_TYPE_INT_8, true));
    HirTypeId t2 = hir_add_type(hir, hir_type_new_int(HIR_TYPE_INT_8, true));
    HirTypeId t3 = hir_add_type(hir, hir_type_new_int(HIR_TYPE_INT_16, true));
    CU_ASSERT_TRUE(t1 == t2);
    CU_ASSERT_TRUE(t1 != t3);
    hir_free(hir);
}

static inline HirExpr hir_put_char(HirDeclId decl_id, HirTypeId t_u8, char c) {
    HirExprStepId *args = vec_create_in(mempool, (HirExprStepId)1);
    return hir_expr_new(vec_create_in(mempool,
        hir_expr_step_new_get_decl(decl_id),
        hir_expr_step_new_const(hir_const_new_int(t_u8, c)),
        hir_expr_step_new_call(0, args),
    ));
}

static void test_hir_simple() {
    /*
        extern(putchar) fun putChar(c: u8);

        global fun main() -> i32 {
            putChar('a');
            putChar('\n');
            return 0; 
        }
     */
    Hir *hir = hir_new();

    HirTypeId t_i32 = hir_add_type(hir, hir_type_new_int(HIR_TYPE_INT_32, true));
    HirTypeId t_u8 = hir_add_type(hir, hir_type_new_int(HIR_TYPE_INT_8, false));
    HirTypeId t_void = hir_add_type(hir, hir_type_new_void());

    HirTypeId t_put_char = hir_add_type(hir, hir_type_new_function(vec_create_in(mempool, t_u8), t_void));
    HirTypeId t_main = hir_add_type(hir, hir_type_new_function(vec_new_in(mempool, HirTypeId), t_i32));
    
    HirDeclId d_put_char = hir_add_decl(hir);
    HirDeclId d_main = hir_add_decl(hir);

    HirExternId e_put_char = hir_add_extern(hir, slice_from_cstr("putchar"),
        hir_extern_info_new(HIR_EXTERN_FUNC, t_put_char));
    HirFuncId f_main = hir_register_fun(hir, t_main);

    hir_init_decl_extern(hir, d_put_char, e_put_char);
    hir_init_decl_func(hir, d_main, f_main);

    HirExpr return_expr = hir_expr_new(vec_create_in(mempool, hir_expr_step_new_const(hir_const_new_int(t_i32, 0))));

    hir_init_fun(hir, f_main, vec_new_in(mempool, HirMutability), hir_func_info_new(mempool,
        opt_slice_new_value(slice_from_cstr("main"))));
    hir_init_fun_body(hir, f_main, hir_code_new(mempool, vec_create_in(mempool, 
            hir_stmt_new_expr(hir_put_char(d_put_char, t_u8, 'a')),
            hir_stmt_new_expr(hir_put_char(d_put_char, t_u8, '\n')),
            hir_stmt_new_ret(return_expr)
        )));

    hir_postprocess(hir);
    LlvmModule *module = llvm_module_new();
    llvm_module_emit(module, hir);
    llvm_module_write_ir(module, "test.ll");
    llvm_module_write_obj(module, "test.o");
    llvm_module_free(module);
    hir_free(hir);
}

void test_hir() {
    CU_pSuite suite = CU_add_suite("hir", on_test_setup, on_test_end);
    CU_ADD_TEST(suite, test_hir_add_type);
    CU_ADD_TEST(suite, test_hir_simple);
}

