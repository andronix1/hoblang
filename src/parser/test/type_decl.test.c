#include "type_decl.test.h"
#include "ast/node.h"
#include "common.test.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

static void test_parser_type_decl_ident() {
    check_parsing(
        vec_create_in(mempool,
                ast_node_new_type_decl(mempool, false,
                    slice_from_cstr("MyCustomType"),
                    create_type("super")
                )
            ),
        "type MyCustomType = super;"
    );
}

static void test_parser_type_decl_path() {
    check_parsing(
        vec_create_in(mempool,
                ast_node_new_type_decl(mempool, false,
                    slice_from_cstr("MyCustomType"),
                    create_type("super.cool")
                )
            ),
        "type MyCustomType = super.cool;"
    );
}

static void test_parser_type_decl_struct_with_trailing_comma() {
    check_parsing(
        vec_create_in(mempool,
                ast_node_new_type_decl(mempool, false,
                    slice_from_cstr("MyCustomType"),
                    ast_type_new_struct(mempool, vec_create_in(mempool,
                        ast_struct_field_new(false, slice_from_cstr("a"), create_type("i32")),
                        ast_struct_field_new(false, slice_from_cstr("b"), create_type("mod.i64"))
                    )))
            ),
        "type MyCustomType = struct {\n"
        "    a: i32,\n"
        "    b: mod.i64,\n"
        "};"
    );
}

static void test_parser_type_decl_struct() {
    check_parsing(
        vec_create_in(mempool,
                ast_node_new_type_decl(mempool, false,
                    slice_from_cstr("MyCustomType"),
                    ast_type_new_struct(mempool, vec_create_in(mempool,
                        ast_struct_field_new(false, slice_from_cstr("a"), create_type("i32")),
                        ast_struct_field_new(false, slice_from_cstr("b"), create_type("mod.i64"))
                    )))
            ),
        "type MyCustomType = struct {\n"
        "    a: i32,\n"
        "    b: mod.i64\n"
        "};"
    );
}

static void test_parser_type_decl_struct_with_local_fields() {
    check_parsing(
        vec_create_in(mempool,
                ast_node_new_type_decl(mempool, false,
                    slice_from_cstr("MyCustomType"),
                    ast_type_new_struct(mempool, vec_create_in(mempool,
                        ast_struct_field_new(true, slice_from_cstr("a"), create_type("i32")),
                        ast_struct_field_new(false, slice_from_cstr("b"), create_type("mod.i64"))
                    )))
            ),
        "type MyCustomType = struct {\n"
        "    local a: i32,\n"
        "    b: mod.i64\n"
        "};"
    );
}

static void test_parser_type_decl_local() {
    check_parsing(
        vec_create_in(mempool,
                ast_node_new_type_decl(mempool, true,
                    slice_from_cstr("Type1"),
                    create_type("t1")
                ),
            ),
        "local type Type1 = t1;\n"
    );
}

void test_parser_type_decl() {
    CU_pSuite suite = CU_add_suite("parser (type decl)", on_test_setup, on_test_end);
    CU_ADD_TEST(suite, test_parser_type_decl_ident);
    CU_ADD_TEST(suite, test_parser_type_decl_path);
    CU_ADD_TEST(suite, test_parser_type_decl_struct);
    CU_ADD_TEST(suite, test_parser_type_decl_struct_with_trailing_comma);
    CU_ADD_TEST(suite, test_parser_type_decl_struct_with_local_fields);
    CU_ADD_TEST(suite, test_parser_type_decl_local);
}
