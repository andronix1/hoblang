#include <llvm-c/Core.h>
#include "llvm/private.h"
#include "core/vec.h"
#include "ast/private/path.h"
#include "sema/module/private.h"
#include "sema/module/parts/decls/struct/impl.h"
#include "sema/value/private.h"
#include "sema/module/decls/impl.h"
#include "llvm/parts/type.h"
#include "llvm/parts/types/slice.h"

LLVMValueRef llvm_resolve_inner_path(LlvmBackend *llvm, LLVMValueRef value, AstInnerPath *path, SemaValue *from) {
    for (size_t i = 0; i < vec_len(path->segments); i++) {
        SemaInnerPath *segment = &path->segments[i].sema;
        switch (segment->type) {
            case SEMA_INNER_PATH_SIZEOF:
                value = llvm_type_sizeof(llvm, llvm_resolve_type(segment->sizeof_type));
                break;
            case SEMA_INNER_PATH_DEREF:
                if (segment->value->type == SEMA_VALUE_VAR) {
                    value = LLVMBuildLoad2(
                        llvm_builder(llvm),
                        LLVMPointerType(llvm_resolve_type(segment->deref_type), 0),
                        value,
                        "deref"
                    );
                }
                break;
            case SEMA_INNER_PATH_STRUCT_MEMBER: {
                switch (segment->struct_member.member->type) {
                    case SEMA_STRUCT_MEMBER_EXT_FUNC:
                        from->ext_func_handle = value;
                        value = segment->struct_member.member->ext_func->llvm_value;
                        break;
                    case SEMA_STRUCT_MEMBER_FIELD: {
                        LLVMValueRef indices[] = {
                            LLVMConstInt(LLVMInt32Type(), 0, false),
                            LLVMConstInt(LLVMInt32Type(), segment->struct_member.member->field_idx, false)
                        };
                        value = LLVMBuildGEP2(
                            llvm_builder(llvm),
                            llvm_resolve_type(segment->struct_member.of),
                            value,
                            indices, 2,
                            "struct_member"
                        );
                        break;
                    }
                }               
                break;
            }
            case SEMA_INNER_PATH_ARRAY_LEN:
                value = LLVMConstInt(LLVMInt32Type(), segment->array_length, false);
                break;
            case SEMA_INNER_PATH_SLICE_RAW:
                value = llvm_slice_ptr(llvm, llvm_resolve_type(segment->slice_type), value);
                break;
            case SEMA_INNER_PATH_SLICE_LEN:
                value = llvm_slice_len(llvm, llvm_resolve_type(segment->slice_type), value);
                break;
        }
    }
    return value;
}

LLVMValueRef llvm_resolve_value_decl_path(LlvmBackend *llvm, AstDeclPath *path, SemaValue *from) {
	LLVMValueRef result = path->decl->value_decl.llvm_value;
	return result;
}

LLVMValueRef llvm_resolve_path(LlvmBackend *llvm, AstPath *path, SemaValue *from) {
    return llvm_resolve_inner_path(
        llvm,
        llvm_resolve_value_decl_path(llvm, &path->decl_path, from),
        &path->inner_path,
        from
    );
}
