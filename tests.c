#include <CUnit/Basic.h>
#include "core/vec.test.h"
#include "core/keymap.test.h"

int main() {
    if (CU_initialize_registry() != CUE_SUCCESS) {
        return CU_get_error();
    }
    test_vec();
    test_keymap();
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
