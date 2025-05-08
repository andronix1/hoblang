#include "ast/node.h"
#include "core/mempool.h"
#include "core/slice.h"
#include "parser/test/common.test.h"
#include <CUnit/Basic.h>

static void test_parser_fun_decl_global() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_fun_decl(mempool, false,
                slice_from_cstr("test"),
                vec_new_in(mempool, AstFunArg),
                NULL
            )
        ),
        "fun test() {}"
    );
}

static void test_parser_fun_decl_returns() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_fun_decl(mempool, false,
                slice_from_cstr("test"),
                vec_new_in(mempool, AstFunArg),
                create_type("t1.t2")
            )
        ),
        "fun test() -> t1.t2 {}"
    );
}

static void test_parser_fun_decl_args() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_fun_decl(mempool, false,
                slice_from_cstr("test"),
                vec_create_in(mempool,
                    ast_node_fun_arg(slice_from_cstr("a"), create_type("i32")),
                    ast_node_fun_arg(slice_from_cstr("b"), create_type("i64"))
                ),
                create_type("t1.t2")
            )
        ),
        "fun test(a: i32, b: i64) -> t1.t2 {}"
    );
}

static void test_parser_fun_decl_args_with_trailing_comma() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_fun_decl(mempool, false,
                slice_from_cstr("test"),
                vec_create_in(mempool,
                    ast_node_fun_arg(slice_from_cstr("a"), create_type("i32")),
                    ast_node_fun_arg(slice_from_cstr("b"), create_type("i64"))
                ),
                create_type("t1.t2")
            )
        ),
        "fun test(\n"
        "    a: i32,\n"
        "    b: i64,\n"
        ") -> t1.t2 {}"
    );
}

static void test_parser_fun_decl_local() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_fun_decl(mempool, true,
                slice_from_cstr("test"),
                vec_new_in(mempool, AstFunArg),
                NULL
            )
        ),
        "local fun test() {}"
    );
}

void test_parser_fun_decl() {
    CU_pSuite suite = CU_add_suite("parser (func decl)", on_test_setup, on_test_end);
    CU_ADD_TEST(suite, test_parser_fun_decl_global);
    CU_ADD_TEST(suite, test_parser_fun_decl_local);
    CU_ADD_TEST(suite, test_parser_fun_decl_returns);
    CU_ADD_TEST(suite, test_parser_fun_decl_args);
    CU_ADD_TEST(suite, test_parser_fun_decl_args_with_trailing_comma);
}
