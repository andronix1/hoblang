#include "ast/body.h"
#include "ast/global.h"
#include "ast/node.h"
#include "core/mempool.h"
#include "core/slice.h"
#include "parser/test/common.test.h"
#include <CUnit/Basic.h>

static void test_parser_fun_decl_simple() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_fun_decl(mempool,
                NULL,
                ast_fun_info_new(mempool, false,
                    slice_from_cstr("test"),
                    vec_new_in(mempool, AstFunArg),
                    NULL
                ),
                ast_body_new(mempool, vec_new_in(mempool, AstNode*))
            )
        ),
        "fun test() {}"
    );
}

static void test_parser_fun_decl_returns() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_fun_decl(mempool,
                NULL,
                ast_fun_info_new(mempool, false,
                    slice_from_cstr("test"),
                    vec_new_in(mempool, AstFunArg),
                    create_type("t1.t2")
                ),
                ast_body_new(mempool, vec_new_in(mempool, AstNode*))
            )
        ),
        "fun test() -> t1.t2 {}"
    );
}

static void test_parser_fun_decl_args() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_fun_decl(mempool,
                NULL,
                ast_fun_info_new(mempool, false,
                    slice_from_cstr("test"),
                    vec_create_in(mempool,
                        ast_fun_arg_new(slice_from_cstr("a"), create_type("i32")),
                        ast_fun_arg_new(slice_from_cstr("b"), create_type("i64"))
                    ),
                    create_type("t1.t2")
                ),
                ast_body_new(mempool, vec_new_in(mempool, AstNode*))
            )
        ),
        "fun test(a: i32, b: i64) -> t1.t2 {}"
    );
}

static void test_parser_fun_decl_args_with_trailing_comma() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_fun_decl(mempool,
                NULL,
                ast_fun_info_new(mempool, false,
                    slice_from_cstr("test"),
                    vec_create_in(mempool,
                        ast_fun_arg_new(slice_from_cstr("a"), create_type("i32")),
                        ast_fun_arg_new(slice_from_cstr("b"), create_type("i64"))
                    ),
                    create_type("t1.t2")
                ),
                ast_body_new(mempool, vec_new_in(mempool, AstNode*))
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
            ast_node_new_fun_decl(mempool,
                NULL,
                ast_fun_info_new(mempool, true,
                    slice_from_cstr("test"),
                    vec_new_in(mempool, AstFunArg),
                    NULL
                ),
                ast_body_new(mempool, vec_new_in(mempool, AstNode*))
            )
        ),
        "local fun test() {}"
    );
}

static void test_parser_fun_decl_extern() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_external_fun(mempool,
                ast_fun_info_new(mempool, false,
                    slice_from_cstr("test"),
                    vec_new_in(mempool, AstFunArg),
                    create_type("i32")
                ), false, slice_from_cstr("")
            )
        ),
        "extern fun test() -> i32;"
    );
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_external_fun(mempool,
                ast_fun_info_new(mempool, false,
                    slice_from_cstr("test"),
                    vec_new_in(mempool, AstFunArg),
                    create_type("i32")
                ), true, slice_from_cstr("name")
            )
        ),
        "extern(name) fun test() -> i32;"
    );
}

static void test_parser_fun_decl_global() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_fun_decl(mempool,
                ast_global_new(mempool),
                ast_fun_info_new(mempool, false,
                    slice_from_cstr("test"),
                    vec_new_in(mempool, AstFunArg),
                    NULL
                ),
                ast_body_new(mempool, vec_new_in(mempool, AstNode*))
            )
        ),
        "global fun test() {}"
    );
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_fun_decl(mempool,
                ast_global_new_with_alias(mempool, slice_from_cstr("name")),
                ast_fun_info_new(mempool, false,
                    slice_from_cstr("test"),
                    vec_new_in(mempool, AstFunArg),
                    NULL
                ),
                ast_body_new(mempool, vec_new_in(mempool, AstNode*))
            )
        ),
        "global(name) fun test() {}"
    );
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_fun_decl(mempool,
                ast_global_new_with_alias(mempool, slice_from_cstr("name")),
                ast_fun_info_new(mempool, true,
                    slice_from_cstr("test"),
                    vec_new_in(mempool, AstFunArg),
                    NULL
                ),
                ast_body_new(mempool, vec_new_in(mempool, AstNode*))
            )
        ),
        "local global(name) fun test() {}"
    );
}

void test_parser_fun_decl() {
    CU_pSuite suite = CU_add_suite("parser (func decl)", on_test_setup, on_test_end);
    CU_ADD_TEST(suite, test_parser_fun_decl_simple);
    CU_ADD_TEST(suite, test_parser_fun_decl_local);
    CU_ADD_TEST(suite, test_parser_fun_decl_returns);
    CU_ADD_TEST(suite, test_parser_fun_decl_args);
    CU_ADD_TEST(suite, test_parser_fun_decl_args_with_trailing_comma);
    CU_ADD_TEST(suite, test_parser_fun_decl_global);
    CU_ADD_TEST(suite, test_parser_fun_decl_extern);
}
