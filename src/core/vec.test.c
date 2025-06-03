#include "vec.h"
#include "vec.test.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

static void test_vec_push_pop() {
    char *test = vec_new(char);
    CU_ASSERT_EQUAL(vec_len(test), 0);

    vec_push(test, 'a');

    CU_ASSERT_EQUAL(vec_len(test), 1);
    CU_ASSERT_NSTRING_EQUAL(test, "a", 1);

    char b = 'b';
    vec_push_ptr(test, &b); CU_ASSERT_EQUAL(vec_len(test), 2);
    CU_ASSERT_NSTRING_EQUAL(test, "ab", 2);

    vec_push(test, 'c'); CU_ASSERT_EQUAL(vec_len(test), 3);
    CU_ASSERT_NSTRING_EQUAL(test, "abc", 3);

    vec_pop(test); CU_ASSERT_EQUAL(vec_len(test), 2);
    CU_ASSERT_NSTRING_EQUAL(test, "ab", 2);

    vec_pop(test); CU_ASSERT_EQUAL(vec_len(test), 1);
    CU_ASSERT_NSTRING_EQUAL(test, "a", 1);

    vec_pop(test); CU_ASSERT_EQUAL(vec_len(test), 0);

    vec_free(test);
}

static void test_vec_remove_at() {
    char *test = vec_new(char);
    vec_push(test, 0);
    vec_push(test, 1);
    vec_push(test, 2);
    vec_push(test, 3);
    vec_remove_at(test, 2);
    CU_ASSERT_EQUAL_FATAL(vec_len(test), 3);
    CU_ASSERT_EQUAL(test[0], 0);
    CU_ASSERT_EQUAL(test[1], 1);
    CU_ASSERT_EQUAL(test[2], 3);
    vec_remove_at(test, 2);
    CU_ASSERT_EQUAL_FATAL(vec_len(test), 2);
    CU_ASSERT_EQUAL(test[0], 0);
    CU_ASSERT_EQUAL(test[1], 1);
    vec_remove_at(test, 0);
    CU_ASSERT_EQUAL_FATAL(vec_len(test), 1);
    CU_ASSERT_EQUAL(test[1], 1);
    vec_free(test);
}

static void test_vec_extends() {
    char *test1 = vec_new(char);
    vec_push(test1, 0);
    vec_push(test1, 1);
    vec_push(test1, 2);
    char *test2 = vec_new(char);
    vec_push(test2, 3);
    vec_push(test2, 4);
    vec_extend(test1, test2);
    CU_ASSERT_EQUAL_FATAL(vec_len(test1), 5);
    for (char i = 0; i < 5; i++) {
        CU_ASSERT_EQUAL(test1[(size_t)i], i);
    }
    vec_free(test1);
    vec_free(test2);
}

static void test_vec_at() {
    char *test = vec_new(char);
    vec_push(test, 0);
    vec_push(test, 0);
    vec_push(test, 0);
    CU_ASSERT_EQUAL(vec_at(test, 0), &test[0]);
    CU_ASSERT_EQUAL(vec_at(test, 1), &test[1]);
    vec_free(test);
}

void test_vec() {
    CU_pSuite suite = CU_add_suite("vec", NULL, NULL);
    CU_ADD_TEST(suite, test_vec_push_pop);
    CU_ADD_TEST(suite, test_vec_at);
    CU_ADD_TEST(suite, test_vec_remove_at);
    CU_ADD_TEST(suite, test_vec_extends);
}
