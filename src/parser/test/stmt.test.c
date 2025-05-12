#include "stmt.test.h"
#include "ast/expr.h"
#include "ast/node.h"
#include "ast/stmt.h"
#include "core/slice.h"
#include "parser/test/common.test.h"

#include <CUnit/Basic.h>

static Slice $;

void test_parser_stmt_expr() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_stmt(mempool,
                ast_stmt_new_expr(mempool, ast_expr_new_path(mempool, $, create_path("sudo.super.path")))
            )
        ),
        "sudo.super.path;"
    );
}

void test_parser_stmt_return() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_stmt(mempool,
                ast_stmt_new_return(mempool, $, ast_expr_new_path(mempool, $, create_path("sudo.super.path")))
            )
        ),
        "return sudo.super.path;"
    );
}

void test_parser_stmt() {
    CU_pSuite suite = CU_add_suite("parser (statements)", on_test_setup, on_test_end);
    CU_ADD_TEST(suite, test_parser_stmt_expr);
    CU_ADD_TEST(suite, test_parser_stmt_return);
}
