#include "file_content.test.h"
#include "core/file_content.h"
#include "core/slice.h"
#include <CUnit/CUnit.h>

static void test_file_content_read() {
    CU_ASSERT_EQUAL_FATAL(file_content_read("for_tests/non_existent_file.txt"), NULL);

    FileContent *content = file_content_read("for_tests/test.txt");
    CU_ASSERT_NOT_EQUAL_FATAL(content, NULL);
    CU_ASSERT_STRING_EQUAL(content->path, "for_tests/test.txt");
    const char* expects = "hello, how are you?\n";
    CU_ASSERT_EQUAL_FATAL(content->data.length, strlen(expects));
    CU_ASSERT_NSTRING_EQUAL(content->data.value, expects, content->data.length);
    file_content_free(content);
}

static void test_file_content_in_memory() {
    FileContent *content = file_content_new_in_memory("hello, how are you?\n");
    CU_ASSERT_NOT_EQUAL_FATAL(content, NULL);
    CU_ASSERT_STRING_EQUAL(content->path, "<memory>");
    const char* expects = "hello, how are you?\n";
    CU_ASSERT_EQUAL_FATAL(content->data.length, strlen(expects));
    CU_ASSERT_NSTRING_EQUAL(content->data.value, expects, content->data.length);
    file_content_free(content);
}

static inline void CU_assert_file_loc_eq(FileLoc a, FileLoc b) {
    CU_ASSERT_EQUAL(a.begin.line, b.begin.line);
    CU_ASSERT_EQUAL(a.begin.character, b.begin.character);
    CU_ASSERT_EQUAL(a.end.line, b.end.line);
    CU_ASSERT_EQUAL(a.end.character, b.end.character);
}

static void test_file_content_locate_multiline() {
    FileContent *content = file_content_new_in_memory("hello, how are you?\ni love you)\n");

    Slice slice = slice_new(&content->data.value[15], 6);
    CU_ASSERT_NSTRING_EQUAL_FATAL(slice.value, "you?\ni", slice.length);

    FileLoc loc = file_content_locate(content, slice);
    FileLoc expects = {
        .begin = { 1, 16 },
        .end = { 2, 1 },
    };
    CU_assert_file_loc_eq(loc, expects);

    file_content_free(content);
}
static void test_file_content_locate() {
    FileContent *content = file_content_new_in_memory("hello, how are you?\ni love you)\n");
    CU_ASSERT_NOT_EQUAL_FATAL(content, NULL);

    Slice slice = slice_new(&content->data.value[22], 4);
    CU_ASSERT_NSTRING_EQUAL_FATAL(slice.value, "love", slice.length);

    FileLoc loc = file_content_locate(content, slice);
    FileLoc expects = {
        .begin = { 2, 3 },
        .end = { 2, 6 },
    };
    CU_assert_file_loc_eq(loc, expects);

    file_content_free(content);
}

static void test_file_content_get_lines() {
    FileContent *content = file_content_new_in_memory("hello, how are you?\ni love you)\nthis is line3\n");
    CU_ASSERT_NOT_EQUAL_FATAL(content, NULL);

    CU_ASSERT(slice_eq(
        file_content_get_lines(content, 1, 1).content,
        slice_from_cstr("hello, how are you?")
    ));
    CU_ASSERT(slice_eq(
        file_content_get_lines(content, 1, 2).content,
        slice_from_cstr("hello, how are you?\ni love you)")
    ));
    CU_ASSERT(slice_eq(
        file_content_get_lines(content, 4, 4).content,
        slice_from_cstr("")
    ));

    file_content_free(content);
}

void test_file_content() {
    CU_pSuite suite = CU_add_suite("file content", NULL, NULL);
    CU_ADD_TEST(suite, test_file_content_read);
    CU_ADD_TEST(suite, test_file_content_in_memory);
    CU_ADD_TEST(suite, test_file_content_locate);
    CU_ADD_TEST(suite, test_file_content_locate_multiline);
    CU_ADD_TEST(suite, test_file_content_get_lines);
}
