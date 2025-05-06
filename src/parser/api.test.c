#include "api.test.h"
#include "ast/node.h"
#include "ast/path.h"
#include "ast/type.h"
#include "core/mempool.h"
#include "core/slice.h"
#include "lexer/api.h"
#include "parser/api.h"
#include <CUnit/Basic.h>

void test_parser_type() {
    Mempool *mempool = mempool_new(1024);
    Parser *parser = parser_new(lexer_new(file_content_new_in_memory(
        "type MyCustomType = struct {"
        "    a: i32,"
        "    b: mod.i64,"
        "}"
    ), true), true);
    AstType *i32 = ast_type_new_path(mempool, ast_path_new(mempool, vec_create_in(mempool,
        ast_path_segment_new_ident(slice_from_cstr("i32"))
    )));
    AstType *mod_i64 = ast_type_new_path(mempool, ast_path_new(mempool, vec_create_in(mempool,
        ast_path_segment_new_ident(slice_from_cstr("mod")),
        ast_path_segment_new_ident(slice_from_cstr("i64")),
    )));
    AstNode *expects = ast_node_new_type_decl(mempool,
        slice_from_cstr("MyCustomType"),
        ast_type_new_struct(mempool, vec_create_in(mempool,
            ast_struct_field_new(slice_from_cstr("a"), i32),
            ast_struct_field_new(slice_from_cstr("b"), mod_i64)
        )));
    parser_free(parser);
    mempool_free(mempool);
}

void test_parser() {
    CU_pSuite suite = CU_add_suite("lexer", NULL, NULL);
    CU_ADD_TEST(suite, test_parser_type);
}
