#include "file_content.test.h"
#include "core/file_content.h"
#include <CUnit/CUnit.h>

static void test_file_content_read() {
    CU_ASSERT_EQUAL_FATAL(file_content_read("for_tests/non_existent_file.txt"), NULL);

    FileContent *content = file_content_read("../for_tests/test.txt");
    CU_ASSERT_NOT_EQUAL_FATAL(content, NULL);
    CU_ASSERT_STRING_EQUAL(content->path, "../for_tests/test.txt");
    const char* expects = "hello, how are you?\n";
    CU_ASSERT_EQUAL_FATAL(content->content.length, strlen(expects));
    CU_ASSERT_NSTRING_EQUAL(content->content.value, expects, content->content.length);
    file_content_free(content);
}

void test_file_content() {
    CU_pSuite suite = CU_add_suite("test file content read", NULL, NULL);
    CU_ADD_TEST(suite, test_file_content_read);
}
