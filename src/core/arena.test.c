#include "arena.test.h"
#include "arena.h"
#include <CUnit/Basic.h>

void test_arena_alloc() {
    Arena *arena = arena_new(10);
    arena_alloc(arena, int);
    __arena_alloc(arena, 1024, 16);
    arena_alloc(arena, int);
    __arena_alloc(arena, 1024, 64);
    __arena_alloc(arena, 12, 64);
    arena_free(arena);
}

void test_arena() {
    CU_pSuite suite = CU_add_suite("file content", NULL, NULL);
    CU_ADD_TEST(suite, test_arena_alloc);
}
