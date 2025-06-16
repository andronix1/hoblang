#include "types.h"
#include "core/assert.h"
#include "ir/ir.h"
#include "llvm/module/module.h"
#include <alloca.h>
#include <llvm-c/Core.h>

static LLVMTypeRef _llvm_ir_type(LlvmModule *module, IrType *type);

static LLVMTypeRef llvm_ir_type(LlvmModule *module, IrTypeId id) {
    if (module->types[id]) return module->types[id];
    IrTypeInfo *info = &module->ir->types[id];
    switch (info->kind) {
        case IR_TYPE_INFO_SIMPLE:
            return module->types[id] = _llvm_ir_type(module, &info->simple);
        case IR_TYPE_INFO_RECORD:
            return module->types[id] = llvm_ir_type(module, info->record.id);
    }
    UNREACHABLE;
}

static LLVMTypeRef _llvm_ir_type(LlvmModule *module, IrType *type) {
    switch (type->kind) {
        case IR_TYPE_VOID: return LLVMVoidTypeInContext(module->context);
        case IR_TYPE_BOOL: return LLVMInt1TypeInContext(module->context);
        case IR_TYPE_INT:
            switch (type->integer.size) {
                case IR_TYPE_INT_8: return LLVMInt8TypeInContext(module->context);
                case IR_TYPE_INT_16: return LLVMInt16TypeInContext(module->context);
                case IR_TYPE_INT_32: return LLVMInt32TypeInContext(module->context);
                case IR_TYPE_INT_64: return LLVMInt64TypeInContext(module->context);
            }
            UNREACHABLE;
        case IR_TYPE_FLOAT:
            switch (type->float_size) {
                case IR_TYPE_FLOAT_32: return LLVMFloatTypeInContext(module->context);
                case IR_TYPE_FLOAT_64: return LLVMDoubleTypeInContext(module->context);
            }
            UNREACHABLE;
        case IR_TYPE_FUNCTION: {
            size_t count = vec_len(type->function.args);
            LLVMTypeRef *types = alloca(sizeof(LLVMTypeRef) * count);
            for (size_t i = 0; i < count; i++) {
                types[i] = llvm_ir_type(module, type->function.args[i]);
            }
            return LLVMFunctionType(llvm_ir_type(module, type->function.returns),
                types, count, false);
        }
        case IR_TYPE_POINTER: return LLVMPointerTypeInContext(module->context, 0);
        case IR_TYPE_ARRAY: return LLVMArrayType2(llvm_ir_type(module, type->array.of), type->array.length);
        case IR_TYPE_STRUCT: {
            size_t count = vec_len(type->structure.fields);
            LLVMTypeRef *types = alloca(sizeof(LLVMTypeRef) * count);
            for (size_t i = 0; i < count; i++) {
                types[i] = llvm_ir_type(module, type->structure.fields[i]);
            }
            return LLVMStructTypeInContext(module->context, types, count, false);
        }
    }
    UNREACHABLE;
}

void llvm_module_setup_types(LlvmModule *module) {
    module->types = vec_new_in(module->mempool, LLVMTypeRef);
    vec_resize(module->types, vec_len(module->ir->types));
    for (size_t i = 0; i < vec_len(module->ir->types); i++) {
        module->types[i] = NULL;
    }
    for (size_t i = 0; i < vec_len(module->ir->types); i++) {
        module->types[i] = llvm_ir_type(module, i);
    }
}
