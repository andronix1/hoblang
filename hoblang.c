#include "core/log.h"
#include "ir/api/ir.h"
#include "ir/stages/stmts.h"
#include "ir/stages/type_tree.h"
#include "print.h"
#include "sema/api.h"
#include "sema/project.h"
#include "llvm/module/api.h"

int main(int argc, char **argv) {
    setup_log();

    if (argc != 2) {
        assert(argc != 0);
        logln("usage: $s <path>", argv[0]);
        return 1;
    }

    Path entry_path = argv[1];

    Ir *ir = ir_new();
    SemaProject *project = sema_project_new(entry_path, ir);
    if (!sema_project_analyze(project)) {
        return 1;
    }
    IrTypeCrossReference *crs = ir_type_check_tree(ir);
    assert(vec_len(crs) == 0);
    ir_fill_stmts(ir);

    LlvmModule *llvm = llvm_module_new();
    llvm_module_emit(llvm, ir);
    llvm_module_write_ir(llvm, "test.ll");
    if (!llvm_module_write_obj(llvm, "test.o")) {
        logln("failed to write module");
        return 1;
    }

    llvm_module_free(llvm);
    sema_project_free(project);
    ir_free(ir);
    return 0;
}
