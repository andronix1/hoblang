#include "api.test.h"
#include "ast/node.h"
#include "core/mempool.h"
#include "core/slice.h"
#include "parser/test/common.test.h"
#include "parser/test/type_decl.test.h"
#include "parser/test/fun_decl.test.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

static void test_parser_multiple_nodes() {
    check_parsing(
        vec_create_in(mempool,
                ast_node_new_type_decl(mempool, false,
                    slice_from_cstr("Type1"),
                    create_type("t1")
                ),
                ast_node_new_type_decl(mempool, false,
                    slice_from_cstr("Type2"),
                    create_type("t2")
                )
            ),
        "type Type1 = t1;\n"
        "type Type2 = t2;\n"
    );
}

void test_parser() {
    test_parser_type_decl();
    test_parser_fun_decl();

    CU_pSuite suite = CU_add_suite("lexer", on_test_setup, on_test_end);
    CU_ADD_TEST(suite, test_parser_multiple_nodes);
}
