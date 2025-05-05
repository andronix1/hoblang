#include "buffer.test.h"
#include "buffer.h"
#include "core/slice.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

static void test_buffer_actions() {
    Buffer buffer = buffer_new();
    buffer_putc(&buffer, 'a'); CU_ASSERT(slice_eq(buffer_value(&buffer), slice_from_cstr("a")))
    buffer_putc(&buffer, 'b'); CU_ASSERT(slice_eq(buffer_value(&buffer), slice_from_cstr("ab")))
    buffer_putc(&buffer, 'c'); CU_ASSERT(slice_eq(buffer_value(&buffer), slice_from_cstr("abc")))
    buffer_erase(&buffer); CU_ASSERT(slice_eq(buffer_value(&buffer), slice_from_cstr("")))
    buffer_free(&buffer);
}

void test_buffer() {
    CU_pSuite suite = CU_add_suite("buffer", NULL, NULL);
    CU_ADD_TEST(suite, test_buffer_actions);
}
