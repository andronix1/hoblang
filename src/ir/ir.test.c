#include "ir.test.h"
#include "ir/api/ir.h"
#include "ir/stages/type_tree.h"
#include <CUnit/Basic.h>

static inline bool ir_type_cross_referenced_in(IrTypeCrossReference *cr, IrTypeId *ids, size_t len) {
    bool found = false;
    for (size_t i = 0; i < len; i++) {
        size_t a = ids[i];
        for (size_t j = i + 1; j < len; j++) {
            size_t b = ids[i];
            if (
                (cr->id == a && cr->what == b) ||
                (cr->id == b && cr->what == a)
            ) {
                found = true;
                break;
            }
        }
        if (found) {
            break;
        }
    }
    return found;
}

static void test_ir_check_type_tree() {
    /*
        type A = struct {
            a: i32,
            b: B;
        };

        type C =  A;

        type B = struct {
            a: C;
        };
    */
    Mempool *mempool = mempool_new(1024);
    Ir *ir = ir_new();
    IrTypeId t_a = ir_add_type_record(ir);
    IrTypeId t_b = ir_add_type_record(ir);
    IrTypeId t_c = ir_add_type_record(ir);
    IrTypeId t_i32 = ir_add_simple_type(ir, ir_type_new_int(IR_TYPE_INT_32, true));
    ir_set_type_record(ir, t_a, 
        ir_add_simple_type(ir, ir_type_new_struct(vec_create_in(mempool, 
            t_i32,
            t_b
        ))));
    ir_set_type_record(ir, t_c, t_a);
    ir_set_type_record(ir, t_b, 
        ir_add_simple_type(ir, ir_type_new_struct(vec_create_in(mempool, 
            t_c
        ))));
    IrTypeCrossReference *crs = ir_type_check_tree(ir);
    CU_ASSERT_EQUAL_FATAL(vec_len(crs), 1);
    size_t exp_crs[] = { t_a, t_b, t_c };
    CU_ASSERT(ir_type_cross_referenced_in(crs, exp_crs, sizeof(exp_crs) / sizeof(exp_crs[0])));
    ir_free(ir);
    mempool_free(mempool);
}

void test_ir() {
    CU_pSuite suite = CU_add_suite("ir", NULL, NULL);
    CU_ADD_TEST(suite, test_ir_check_type_tree);
}
