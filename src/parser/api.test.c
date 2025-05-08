#include "api.test.h"
#include "ast/api/type.h"
#include "ast/node.h"
#include "ast/path.h"
#include "ast/type.h"
#include "core/mempool.h"
#include "core/slice.h"
#include "core/vec.h"
#include "lexer/api.h"
#include "parser/api.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <string.h>

typedef struct {
    AstType *i32;
    AstType *mod_i64;
} TestTypes;

static AstType *create_type(Mempool *mempool, const char *str) {
    AstPathSegment *segments = vec_new_in(mempool, AstPathSegment);
    size_t begin = 0;
    Slice slice = slice_from_cstr(str);
    for (size_t i = 0; str[i] != '\0'; i++) {
        if (str[i] == '.') {
            vec_push(segments, ast_path_segment_new_ident(subslice(slice, begin, i)));
            begin = i + 1;
        }
    }
    vec_push(segments, ast_path_segment_new_ident(subslice(slice, begin, slice.length)));
    return ast_type_new_path(mempool, ast_path_new(mempool, segments));
}

static void check_parsing(AstNode **expects, const char *code) {
    Parser *parser = parser_new(lexer_new(file_content_new_in_memory(code), true), true);
    AstNode **actual = parser_parse(parser);
    CU_ASSERT_NOT_EQUAL_FATAL(actual, NULL);
    CU_ASSERT_EQUAL(vec_len(actual), vec_len(expects));
    if (vec_len(actual) != vec_len(expects)) {
        return;
    }
    for (size_t i = 0; i < vec_len(actual); i++) {
        CU_ASSERT(ast_node_eq(expects[i], actual[i]));
    }
    parser_free(parser);
}

static void test_parser_type_decl_ident() {
    Mempool *mempool = mempool_new(1024);
    check_parsing(
        vec_create_in(mempool,
                ast_node_new_type_decl(mempool,
                    slice_from_cstr("MyCustomType"),
                    create_type(mempool, "super")
                )
            ),
        "type MyCustomType = super"
    );
    mempool_free(mempool);
}

static void test_parser_type_decl_path() {
    Mempool *mempool = mempool_new(1024);
    check_parsing(
        vec_create_in(mempool,
                ast_node_new_type_decl(mempool,
                    slice_from_cstr("MyCustomType"),
                    create_type(mempool, "super.cool")
                )
            ),
        "type MyCustomType = super.cool"
    );
    mempool_free(mempool);
}

static void test_parser_type_decl_struct_with_trailing_comma() {
    Mempool *mempool = mempool_new(1024);
    check_parsing(
        vec_create_in(mempool,
                ast_node_new_type_decl(mempool,
                    slice_from_cstr("MyCustomType"),
                    ast_type_new_struct(mempool, vec_create_in(mempool,
                        ast_struct_field_new(slice_from_cstr("a"), create_type(mempool, "i32")),
                        ast_struct_field_new(slice_from_cstr("b"), create_type(mempool, "mod.i64"))
                    )))
            ),
        "type MyCustomType = struct {\n"
        "    a: i32,\n"
        "    b: mod.i64,\n"
        "}"
    );
    mempool_free(mempool);
}

static void test_parser_type_decl_struct() {
    Mempool *mempool = mempool_new(1024);
    check_parsing(
        vec_create_in(mempool,
                ast_node_new_type_decl(mempool,
                    slice_from_cstr("MyCustomType"),
                    ast_type_new_struct(mempool, vec_create_in(mempool,
                        ast_struct_field_new(slice_from_cstr("a"), create_type(mempool, "i32")),
                        ast_struct_field_new(slice_from_cstr("b"), create_type(mempool, "mod.i64"))
                    )))
            ),
        "type MyCustomType = struct {\n"
        "    a: i32,\n"
        "    b: mod.i64\n"
        "}"
    );
    mempool_free(mempool);
}

void test_parser() {
    CU_pSuite suite = CU_add_suite("lexer", NULL, NULL);
    CU_ADD_TEST(suite, test_parser_type_decl_ident);
    CU_ADD_TEST(suite, test_parser_type_decl_path);
    CU_ADD_TEST(suite, test_parser_type_decl_struct);
    CU_ADD_TEST(suite, test_parser_type_decl_struct_with_trailing_comma);
}
