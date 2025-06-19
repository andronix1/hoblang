#include "types.h"
#include "core/assert.h"
#include "hir/hir.h"
#include "llvm/module/module.h"
#include <alloca.h>
#include <llvm-c/Core.h>

LLVMTypeRef llvm_function_type(LlvmModule *module, HirTypeId type_id) {
    HirType *type = hir_resolve_simple_type(module->hir, type_id);
    assert(type->kind == HIR_TYPE_FUNCTION);
    size_t count = vec_len(type->function.args);
    LLVMTypeRef *types = alloca(sizeof(LLVMTypeRef) * count);
    for (size_t i = 0; i < count; i++) {
        types[i] = llvm_runtime_type(module, type->function.args[i]);
    }
    return LLVMFunctionType(llvm_runtime_type(module, type->function.returns), types, count, false);
}

LLVMTypeRef llvm_runtime_type(LlvmModule *module, HirTypeId type_id) {
    HirType *type = hir_resolve_simple_type(module->hir, type_id);
    switch (type->kind) {
        case HIR_TYPE_VOID: return LLVMVoidTypeInContext(module->context);
        case HIR_TYPE_BOOL: return LLVMInt1TypeInContext(module->context);
        case HIR_TYPE_INT:
            switch (type->integer.size) {
                case HIR_TYPE_INT_8: return LLVMInt8TypeInContext(module->context);
                case HIR_TYPE_INT_16: return LLVMInt16TypeInContext(module->context);
                case HIR_TYPE_INT_32: return LLVMInt32TypeInContext(module->context);
                case HIR_TYPE_INT_64: return LLVMInt64TypeInContext(module->context);
            }
            UNREACHABLE;
        case HIR_TYPE_FLOAT:
            switch (type->float_size) {
                case HIR_TYPE_FLOAT_32: return LLVMFloatTypeInContext(module->context);
                case HIR_TYPE_FLOAT_64: return LLVMDoubleTypeInContext(module->context);
            }
            UNREACHABLE;
        case HIR_TYPE_FUNCTION: 
        case HIR_TYPE_POINTER:
            return LLVMPointerTypeInContext(module->context, 0);
        case HIR_TYPE_ARRAY: return LLVMArrayType2(llvm_runtime_type(module, type->array.of), type->array.length);
        case HIR_TYPE_STRUCT: {
            size_t count = vec_len(type->structure.fields);
            LLVMTypeRef *types = alloca(sizeof(LLVMTypeRef) * count);
            for (size_t i = 0; i < count; i++) {
                types[i] = llvm_runtime_type(module, type->structure.fields[i].type);
            }
            return LLVMStructTypeInContext(module->context, types, count, false);
        }

        case HIR_TYPE_GEN_PARAM: case HIR_TYPE_GEN: TODO;
    }
    UNREACHABLE;
}
