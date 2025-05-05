#include "api.test.h"
#include "core/file_content.h"
#include "core/slice.h"
#include "lexer/api.h"
#include "lexer/token.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

static void test_lexer_from_file() {
    CU_ASSERT_EQUAL_FATAL(lexer_from_file("for_tests/non_existent_file.hob"), NULL);

    Lexer *lexer = lexer_from_file("for_tests/tokens.hob");
    CU_ASSERT_NOT_EQUAL_FATAL(lexer, NULL)
    lexer_free(lexer);
}

static void CU_assert_tokens_eq(Token a, Token b) {
    CU_ASSERT_EQUAL(a.kind, b.kind);
}

static void test_lexer_next_token() {
    FileContent *content = file_content_new_in_memory("   \t \n +-+=-=hello");
    Lexer *lexer = lexer_from_file_content(content);

    CU_assert_tokens_eq(lexer_next(lexer), token_simple(TOKEN_PLUS));
    CU_assert_tokens_eq(lexer_next(lexer), token_simple(TOKEN_MINUS));
    CU_assert_tokens_eq(lexer_next(lexer), token_simple(TOKEN_APPEND));
    CU_assert_tokens_eq(lexer_next(lexer), token_simple(TOKEN_SUBTRACT));

    Token ident = lexer_next(lexer);
    CU_assert_tokens_eq(ident, token_simple(TOKEN_IDENT));
    CU_ASSERT(slice_eq(slice_from_cstr("hello"), ident.slice))

    CU_assert_tokens_eq(lexer_next(lexer), token_simple(TOKEN_EOI));
    CU_assert_tokens_eq(lexer_next(lexer), token_simple(TOKEN_EOI));

    lexer_free(lexer);
    file_content_free(content);
}

void test_lexer() {
    CU_pSuite suite = CU_add_suite("lexer", NULL, NULL);
    CU_ADD_TEST(suite, test_lexer_from_file);
    CU_ADD_TEST(suite, test_lexer_next_token);
}
