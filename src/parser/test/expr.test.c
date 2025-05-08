#include "expr.test.h"
#include "ast/api/expr.h"
#include "ast/expr.h"
#include "ast/node.h"
#include "ast/stmt.h"
#include "core/mempool.h"
#include "parser/test/common.test.h"
#include <CUnit/Basic.h>

void test_parser_expr_integer() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_stmt(mempool,
                ast_stmt_new_expr(mempool, ast_expr_new_integer(mempool, 100))
            )
        ),
        "100;"
    );
}

void test_parser_expr_path() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_stmt(mempool,
                ast_stmt_new_expr(mempool, ast_expr_new_path(mempool, create_path("asd.asd")))
            )
        ),
        "asd.asd;"
    );
}

void test_parser_expr_scope() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_stmt(mempool,
                ast_stmt_new_expr(mempool, ast_expr_new_path(mempool, create_path("asd.asd")))
            )
        ),
        "(asd.asd);"
    );
}

void test_parser_expr_call() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_stmt(mempool,
                ast_stmt_new_expr(mempool,
                    ast_expr_new_callable(mempool,
                        ast_expr_new_path(mempool, create_path("func")),
                        vec_create_in(mempool,
                            ast_expr_new_path(mempool, create_path("a")),
                            ast_expr_new_path(mempool, create_path("b")),
                            ast_expr_new_path(mempool, create_path("c"))
                        )
                    )
                )
            )
        ),
        "func(a, b, c);"
    );
}

void test_parser_expr_call_with_trailing_comma() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_stmt(mempool,
                ast_stmt_new_expr(mempool,
                    ast_expr_new_callable(mempool,
                        ast_expr_new_path(mempool, create_path("func")),
                        vec_create_in(mempool,
                            ast_expr_new_path(mempool, create_path("a")),
                            ast_expr_new_path(mempool, create_path("b")),
                            ast_expr_new_path(mempool, create_path("c"))
                        )
                    )
                )
            )
        ),
        "func(a, b, c,);"
    );
}

void test_parser_expr_call_without_args() {
    check_parsing(
        vec_create_in(mempool,
            ast_node_new_stmt(mempool,
                ast_stmt_new_expr(mempool,
                    ast_expr_new_callable(mempool,
                        ast_expr_new_path(mempool, create_path("func")),
                        vec_new_in(mempool, AstExpr*)
                    )
                )
            )
        ),
        "func();"
    );
}

void test_parser_expr() {
    CU_pSuite suite = CU_add_suite("parser (expression)", on_test_setup, on_test_end);
    CU_ADD_TEST(suite, test_parser_expr_integer);
    CU_ADD_TEST(suite, test_parser_expr_path);
    CU_ADD_TEST(suite, test_parser_expr_scope);
    CU_ADD_TEST(suite, test_parser_expr_call);
    CU_ADD_TEST(suite, test_parser_expr_call_with_trailing_comma);
    CU_ADD_TEST(suite, test_parser_expr_call_without_args);
}
