#include <llvm-c/Core.h>
#include "llvm/private.h"
#include "core/vec.h"
#include "ast/private/path.h"
#include "ast/private/module_node.h"
#include "sema/module/parts/decls/struct/impl.h"
#include "sema/value/private.h"
#include "sema/module/decls/impl.h"
#include "llvm/parts/type.h"
#include "llvm/parts/types/slice.h"
#include "llvm/parts/types/optional.h"
#include "llvm/utils/member.h"

LLVMValueRef llvm_resolve_path(LlvmBackend *llvm, LLVMValueRef value, AstPath *path, SemaValue *from) {
    for (size_t i = 0; i < vec_len(path->segments); i++) {
        SemaPath *segment = &path->segments[i].sema;
        switch (segment->type) {
            case SEMA_PATH_DECL: {
                value = segment->decl->llvm.value;
                break;
            }
            case SEMA_PATH_SIZEOF:
                value = llvm_type_sizeof(llvm, llvm_resolve_type(segment->sizeof_op.type), llvm_resolve_type(segment->sizeof_op.output_type));
                break;
            case SEMA_PATH_DEREF:
                if (segment->value->type == SEMA_VALUE_VAR) {
                    value = LLVMBuildLoad2(
                        llvm_builder(llvm),
                        LLVMPointerType(llvm_resolve_type(segment->deref_type), 0),
                        value,
                        "deref"
                    );
                }
                break;
            case SEMA_PATH_STRUCT_MEMBER: {
                switch (segment->struct_member.member->type) {
                    case SEMA_STRUCT_MEMBER_EXT_FUNC:
                        from->ext_func_handle = value;
                        if (!segment->struct_member.member->ext_func.is_ptr && LLVMGetTypeKind(LLVMTypeOf(value)) == LLVMPointerTypeKind) {
                            from->ext_func_handle = LLVMBuildLoad2(
                                llvm_builder(llvm),
                                llvm_resolve_type(segment->struct_member.of),
                                value,
                                ""
                            );
                        }
                        value = segment->struct_member.member->ext_func.decl->llvm.value;
                        break;
                    case SEMA_STRUCT_MEMBER_FIELD: {
                        size_t mid = segment->struct_member.member->field_idx;
                        value = llvm_get_member(
                            llvm,
                            llvm_resolve_type(segment->struct_member.of),
                            llvm_resolve_type(segment->struct_member.of->struct_def->members[mid].type->sema),
                            value,
                            mid,
                            false
                        );
                        break;
                    }
                }               
                break;
            }
            case SEMA_PATH_IS_NULL:
                value = llvm_opt_is_null(llvm, llvm_resolve_type(segment->optional_type), value, false);
                break;
            case SEMA_PATH_ARRAY_LEN:
                value = LLVMConstInt(LLVMInt32Type(), segment->array_length, false);
                break;
            case SEMA_PATH_SLICE_RAW:
                value = llvm_slice_ptr(llvm, llvm_resolve_type(segment->slice_type), value, false);
                break;
            case SEMA_PATH_SLICE_LEN:
                value = llvm_slice_len(llvm, llvm_resolve_type(segment->slice_type), value, false);
                break;
        }
    }
    return value;
}
