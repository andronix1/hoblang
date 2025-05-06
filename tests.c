#include <CUnit/Basic.h>
#include "core/arena.test.h"
#include "core/buffer.test.h"
#include "core/file_content.test.h"
#include "core/mempool.test.h"
#include "core/vec.test.h"
#include "core/keymap.test.h"
#include "lexer/api.test.h"
#include "parser/api.test.h"

static void setup_tests() {
    test_vec();
    test_keymap();
    test_buffer();
    test_arena();
    test_mempool();
    test_file_content();
    test_lexer();
    test_parser();
}

int main() {
    if (CU_initialize_registry() != CUE_SUCCESS) {
        return CU_get_error();
    }
    setup_tests();
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
