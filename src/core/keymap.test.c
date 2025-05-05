#include "keymap.h"
#include "core/slice.h"
#include "keymap.test.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

static void test_keymap_insert() {
    char *test = keymap_new(char);

    CU_ASSERT_EQUAL_FATAL(keymap_insert(test, slice_from_cstr("lol"), 'a'), NULL);
    CU_ASSERT_EQUAL_FATAL(keymap_insert(test, slice_from_cstr("lol2"), 'n'), NULL);

    char *dublicated_value = keymap_insert(test, slice_from_cstr("lol"), 'b');
    CU_ASSERT_NOT_EQUAL_FATAL(dublicated_value, NULL);
    CU_ASSERT_EQUAL(*dublicated_value, 'a');

    char *lol = keymap_get(test, slice_from_cstr("lol"));
    CU_ASSERT_NOT_EQUAL_FATAL(lol, NULL);
    CU_ASSERT_EQUAL(*lol, 'a');

    char *lol2 = keymap_get(test, slice_from_cstr("lol2"));
    CU_ASSERT_NOT_EQUAL_FATAL(lol2, NULL);
    CU_ASSERT_EQUAL(*lol2, 'n');

    keymap_free(test);
}

void test_keymap() {
    CU_pSuite suite = CU_add_suite("keymap", NULL, NULL);
    CU_ADD_TEST(suite, test_keymap_insert);
}
