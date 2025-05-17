#include "path.h"
#include "ast/path.h"
#include "sema/module/nodes/path_ext.h"
#include "sema/module/module.h"
#include "llvm/module/value.h"
#include <stdio.h>

static LLVMValueRef llvm_emit_path_from(LlvmModule *module, AstPath *path, LLVMValueRef value) {
    for (size_t i = 0; i < vec_len(path->segments); i++) {
        SemaPathSegmentExt *segment = &path->segments[i].sema;
        switch (segment->kind) {
            case SEMA_PATH_SEGMENT_IGNORE:
                break;
            case SEMA_PATH_SEGMENT_DECL:
                value = segment->decl->llvm.value;
                break;
            case SEMA_PATH_SEGMENT_STRUCT_FIELD:
                value = llvm_get_value(module, value, segment->struct_field_idx, segment->from_value);
                break;
            case SEMA_PATH_SEGMENT_EXT_DIRECT:
                // TODO: optional load
                segment->ext.handle->llvm.value = llvm_opt_load(module, value, segment->from_value);
                value = segment->ext.decl->llvm.value;
                break;
            case SEMA_PATH_SEGMENT_EXT_REF:
                segment->ext.handle->llvm.value = value;
                value = segment->ext.decl->llvm.value;
                break;
        }
    }
    assert(value);
    return value;
}

LLVMValueRef llvm_emit_path(LlvmModule *module, AstPath *path) {
    return llvm_emit_path_from(module, path, NULL);
}
