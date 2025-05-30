#include "common.test.h"
#include "ast/api/node.h"
#include "ast/api/path.h"
#include "ast/api/type.h"
#include "ast/expr.h"
#include "ast/node.h"
#include "ast/path.h"
#include "ast/type.h"
#include "parser/api.h"
#include <CUnit/CUnit.h>

Mempool *mempool;

AstPath *create_path(const char *str) {
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
    return ast_path_new(mempool, segments);
}

static Slice $;

AstExpr *create_path_expr(const char *str) {
    return ast_expr_new_path(mempool, $, create_path(str));
}

AstType *create_type(const char *str) {
    return ast_type_new_path(mempool, create_path(str));
}

void check_parsing(AstNode **expects, const char *code) {
    Parser *parser = parser_new(lexer_new(file_content_new_in_memory(code)));
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

int on_test_setup() {
    mempool = mempool_new(1024);
    return 0;
}

int on_test_end() {
    mempool_free(mempool);
    return 0;
}
