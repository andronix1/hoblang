#include "core/file_content.h"
#include "core/log.h"
#include "ir/api/ir.h"
#include "ir/stages/checks.h"
#include "ir/stages/stmts.h"
#include "ir/stages/type_tree.h"
#include "lexer/api.h"
#include "parser/api.h"
#include "print.h"
#include "sema/module/api/module.h"
#include "sema/module/stages/setup.h"
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
    SemaModule *module = sema_module_new(ir, parser_new(lexer_new(file_content_read(entry_path))));
    if (!module) {
        return 1;
    }
    sema_module_setup(module);
    sema_module_emit(module);

    if (sema_module_failed(module)) {
        sema_module_free(module);
        ir_free(ir);
        return 1;
    }

    IrTypeCrossReference *crs = ir_type_check_tree(ir);
    assert(vec_len(crs) == 0);
    ir_fill_stmts(ir);
    ir_check_cosistency(ir);

    LlvmModule *llvm = llvm_module_new();
    llvm_module_emit(llvm, ir);
    llvm_module_write_ir(llvm, "test.ll");
    if (!llvm_module_write_obj(llvm, "test.o")) {
        logln("failed to write module");
        return 1;
    }

    sema_module_free(module);
    llvm_module_free(llvm);
    ir_free(ir);
    return 0;
}
