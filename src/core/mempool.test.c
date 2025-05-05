#include "mempool.test.h"
#include "mempool.h"
#include <CUnit/Basic.h>

void test_mempool_alloc() {
    Mempool *mempool = mempool_new(1);
    vec_new_in(mempool, int);
    vec_new_in(mempool, void*);
    vec_new_in(mempool, char);
    vec_new_in(mempool, typeof(mempool));
    keymap_new_in(mempool, int);
    keymap_new_in(mempool, void*);
    keymap_new_in(mempool, char);
    keymap_new_in(mempool, typeof(mempool));
    mempool_alloc(mempool, int);
    mempool_alloc(mempool, void*);
    mempool_alloc(mempool, char);
    mempool_alloc(mempool, typeof(mempool));
    mempool_free(mempool);
}

void test_mempool() {
    CU_pSuite suite = CU_add_suite("mempool", NULL, NULL);
    CU_ADD_TEST(suite, test_mempool_alloc);
}
