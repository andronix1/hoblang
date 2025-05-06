#include "api.test.h"
#include "lexer/api.h"
#include "parser/api.h"
#include <CUnit/Basic.h>

void test_parser_type() {
    Parser *parser = parser_new(lexer_new(file_content_new_in_memory(
        "type MyCustomType = struct {"
        "    a: i32,"
        "    b: i32,"
        "}"
    ), true), true);
    parser_free(parser);
}

void test_parser() {
    CU_pSuite suite = CU_add_suite("lexer", NULL, NULL);
    CU_ADD_TEST(suite, test_parser_type);
}
