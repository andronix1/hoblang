#include "path.h"
#include "ast/path.h"
#include "sema/module/nodes/path_ext.h"
#include "sema/module/module.h"

static LLVMValueRef llvm_emit_path_from(LlvmModule *module, AstPath *path, LLVMValueRef value) {
    for (size_t i = 0; i < vec_len(path->segments); i++) {
        SemaPathSegmentExt *segment = &path->segments->sema;
        switch (segment->kind) {
            case SEMA_PATH_SEGMENT_IGNORE:
                break;
            case SEMA_PATH_SEGMENT_DECL:
                value = segment->decl->llvm.value;
                break;
        }
    }
    assert(value);
    return value;
}

LLVMValueRef llvm_emit_path(LlvmModule *module, AstPath *path) {
    return llvm_emit_path_from(module, path, NULL);
}
