#include "type.h"
#include "core/assert.h"
#include "core/keymap.h"
#include "core/vec.h"
#include "sema/module/api/value.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include <alloca.h>
#include <llvm-c/Core.h>
#include <llvm-c/Types.h>

LLVMTypeRef llvm_type(LlvmModule *module, SemaType *type) {
    switch (type->kind) {
        case SEMA_TYPE_PRIMITIVE:
            switch (type->primitive.kind) {
                case SEMA_PRIMITIVE_INT:
                    switch (type->primitive.integer.size) {
                        case SEMA_PRIMITIVE_INT8: return LLVMInt8Type();
                        case SEMA_PRIMITIVE_INT16: return LLVMInt16Type();
                        case SEMA_PRIMITIVE_INT32: return LLVMInt32Type();
                        case SEMA_PRIMITIVE_INT64: return LLVMInt64Type();
                    }
                    UNREACHABLE;
                case SEMA_PRIMITIVE_FLOAT:
                    switch (type->primitive.fp) {
                        case SEMA_PRIMITIVE_FLOAT32: return LLVMFloatType();
                        case SEMA_PRIMITIVE_FLOAT64: return LLVMDoubleType();
                    }
                    UNREACHABLE;
                case SEMA_PRIMITIVE_VOID: return LLVMVoidType();
                case SEMA_PRIMITIVE_BOOL: return LLVMInt1Type();
            }
            UNREACHABLE;
        case SEMA_TYPE_FUNCTION: {
            size_t count = vec_len(type->function.args);
            LLVMTypeRef *params = alloca(sizeof(LLVMTypeRef) * count);
            for (size_t i = 0; i < count; i++) {
                params[i] = llvm_type(module, type->function.args[i]);
            }
            return LLVMFunctionType(llvm_type(module, type->function.returns), params, count, false);
        }
        case SEMA_TYPE_STRUCT: {
            size_t count = vec_len(type->structure.fields_map);
            LLVMTypeRef *types = alloca(sizeof(LLVMTypeRef) * count);
            for (size_t i = 0; i < count; i++) {
                keymap_at(type->structure.fields_map, i, field);
                types[i] = llvm_type(module, field->value.type);
            }
            return LLVMStructType(types, count, false);
        }
        case SEMA_TYPE_POINTER:
            return LLVMPointerType(llvm_type(module, type->pointer_to), 0);
        case SEMA_TYPE_SLICE:
            // TODO: struct
            return LLVMPointerType(llvm_type(module, type->slice_of), 0);
        case SEMA_TYPE_GENERIC: UNREACHABLE;
    }
    UNREACHABLE;
}

LLVMTypeRef llvm_decl_type(LlvmModule *module, SemaDeclHandle *handle) {
    return llvm_type(module, sema_value_is_runtime(handle->value));
}
