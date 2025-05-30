#include "api.test.h"
#include "core/file_content.h"
#include "lexer/api.h"
#include "parser/api.h"
#include "sema/module/api.h"
#include "sema/module/module.h"
#include <CUnit/Basic.h>

static void test_sema_create() {
    SemaModule *module = sema_module_new(parser_new(lexer_new(file_content_new_in_memory(""))), NULL);
    sema_module_free(module);
}

void test_sema() {
    CU_pSuite suite = CU_add_suite("sema", NULL, NULL);
    CU_ADD_TEST(suite, test_sema_create);
}
