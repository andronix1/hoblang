#include "value_decl.test.h"
#include "ast/expr.h"
#include "ast/node.h"
#include "common.test.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

static Slice $;

static void test_parser_value_decl_kind() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_value_decl(mempool,
                NULL,
                ast_value_info_new(mempool, false,
                    AST_VALUE_DECL_VAR, slice_from_cstr("myVar"),
                    NULL
                ),
                NULL
            ),
            ast_node_new_value_decl(mempool,
                NULL,
                ast_value_info_new(mempool, false,
                    AST_VALUE_DECL_FINAL, slice_from_cstr("myFinal"),
                    NULL
                ),
                NULL
            ),
            ast_node_new_value_decl(mempool,
                NULL,
                ast_value_info_new(mempool, false,
                    AST_VALUE_DECL_CONST, slice_from_cstr("myConst"),
                    NULL
                ),
                NULL
            ),
        ),
        "var myVar;\n"
        "final myFinal;\n"
        "const myConst;\n"
    );
}

static void test_parser_value_decl_explicit_type() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_value_decl(mempool,
                NULL,
                ast_value_info_new(mempool,
                    false,
                    AST_VALUE_DECL_CONST, slice_from_cstr("myConst"),
                    create_type("i32")
                ),
                NULL
            ),
        ),
        "const myConst: i32;\n"
    );
}

static void test_parser_value_decl_initializer() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_value_decl(mempool,
                NULL,
                ast_value_info_new(mempool,
                    false, AST_VALUE_DECL_CONST,
                    slice_from_cstr("myConst"),
                    NULL
                ),
                ast_expr_new_path(mempool, $, create_path("super"))
            ),
        ),
        "const myConst = super;\n"
    );
}

static void test_parser_value_decl_local() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_value_decl(mempool,
                NULL,
                ast_value_info_new(mempool,
                    true,
                    AST_VALUE_DECL_CONST, slice_from_cstr("myConst"),
                    create_type("i32")
                ),
                ast_expr_new_path(mempool, $, create_path("super"))
            ),
        ),
        "local const myConst: i32 = super;\n"
    );
}

static void test_parser_value_decl_full() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_value_decl(mempool,
                NULL,
                ast_value_info_new(mempool,
                    false,
                    AST_VALUE_DECL_CONST, slice_from_cstr("myConst"),
                    create_type("i32")
                ),
                ast_expr_new_path(mempool, $, create_path("super"))
            ),
        ),
        "const myConst: i32 = super;\n"
    );
}

void test_parser_value_decl() {
    CU_pSuite suite = CU_add_suite("parser (value decl)", on_test_setup, on_test_end);
    CU_ADD_TEST(suite, test_parser_value_decl_kind);
    CU_ADD_TEST(suite, test_parser_value_decl_explicit_type);
    CU_ADD_TEST(suite, test_parser_value_decl_initializer);
    CU_ADD_TEST(suite, test_parser_value_decl_full);
    CU_ADD_TEST(suite, test_parser_value_decl_local);
}
