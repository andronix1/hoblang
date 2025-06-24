#include "expr.h"
#include "core/assert.h"
#include "core/attributes.h"
#include "core/mempool.h"
#include "core/vec.h"
#include "hir/hir.h"
#include "llvm/module/module.h"
#include "llvm/module/types.h"
#include <alloca.h>
#include <assert.h>
#include <llvm-c/Core.h>
#include <stdio.h>

typedef struct {
    LLVMValueRef *values;
    LLVMBasicBlockRef *bbs;
    LLVMBasicBlockRef end;
} LlvmEmitExprCtx;

static inline bool llvm_emit_expr_ctx_is_empty(LlvmEmitExprCtx *ctx) {
    assert(
        (ctx->bbs == NULL) == (ctx->values == NULL) &&
        (ctx->bbs == NULL) == (ctx->end == NULL)
    );
    return ctx->bbs == NULL;
}

static inline size_t llvm_emit_expr_ctx_len(LlvmEmitExprCtx *ctx) {
    if (llvm_emit_expr_ctx_is_empty(ctx)) {
        return 0;
    }
    assert(vec_len(ctx->bbs) == vec_len(ctx->values));
    return vec_len(ctx->bbs);
}

static inline UNUSED LLVMBasicBlockRef llvm_emit_expr_ctx_end(LlvmModule *module, LlvmEmitExprCtx *ctx) {
    if (llvm_emit_expr_ctx_is_empty(ctx)) {
        ctx->bbs = vec_new_in(module->mempool, LLVMBasicBlockRef);
        ctx->values = vec_new_in(module->mempool, LLVMValueRef);
        ctx->end = LLVMAppendBasicBlock(module->func.value, "");
    }
    return ctx->end;
}

static inline void llvm_emit_expr_ctx_append(LlvmModule *module, LlvmEmitExprCtx *ctx, LLVMValueRef value) {
    assert(!llvm_emit_expr_ctx_is_empty(ctx));
    vec_push(ctx->values, value);
    vec_push(ctx->bbs, LLVMGetInsertBlock(module->builder));
}

static inline LlvmEmitExprCtx llvm_emit_expr_ctx_new() {
    LlvmEmitExprCtx ctx = {
        .bbs = NULL,
        .values = NULL,
        .end = NULL,
    };
    return ctx;
}

typedef struct {
    LLVMValueRef value;
    LLVMTypeRef type;
    bool loaded;
} LlvmEmitStepRes;

static inline LlvmEmitStepRes llvm_emit_step_res_new(LLVMValueRef value, bool loaded) {
    LlvmEmitStepRes res = {
        .value = value,     
        .loaded = loaded,     
    };
    return res;
}

static inline LLVMValueRef llvm_get_res_value(LlvmModule *module, LlvmEmitStepRes *res) {
    assert(res->value);
    if (!res->loaded) {
        return LLVMBuildLoad2(module->builder, res->type, res->value, "");
    }
    return res->value;
}

static LLVMValueRef llvm_emit_expr_binop(LlvmModule *module, LlvmEmitStepRes *results, HirBinop *binop) {
    LLVMValueRef ls = llvm_get_res_value(module, &results[binop->ls]);
    LLVMValueRef rs = llvm_get_res_value(module, &results[binop->rs]);
    switch (binop->kind) {
        case HIR_BINOP_COMPARE:
            switch (binop->compare.kind) {
                case HIR_COMPARE_EQ:
                    switch (binop->compare.val_kind) {
                        case HIR_COMPARE_NUMBER:
                        case HIR_COMPARE_BOOL:
                            return LLVMBuildICmp(module->builder, LLVMIntEQ, ls, rs, "");
                    }
                    UNREACHABLE;
                case HIR_COMPARE_NE:
                    switch (binop->compare.val_kind) {
                        case HIR_COMPARE_NUMBER:
                        case HIR_COMPARE_BOOL:
                            return LLVMBuildICmp(module->builder, LLVMIntNE, ls, rs, "");
                    }
                    UNREACHABLE;
            }
            UNREACHABLE;
        case HIR_BINOP_ARITHMETIC:
            #define HIR_BINOP_ARITHM_OPT(INT, UINT, FLOAT, ...) \
                switch (binop->arithm.number.kind) { \
                    case HIR_NUMBER_INT: \
                        return INT(module->builder, ls, rs, ""); \
                    case HIR_NUMBER_UINT: \
                        return UINT(module->builder, ls, rs, ""); \
                    case HIR_NUMBER_FLOAT: \
                        return FLOAT(module->builder, ls, rs, ""); \
                } \
                UNREACHABLE;
            switch (binop->arithm.kind) {
                case HIR_BINOP_ARITHM_ADD:
                    HIR_BINOP_ARITHM_OPT(LLVMBuildAdd, LLVMBuildAdd, LLVMBuildFAdd);
                case HIR_BINOP_ARITHM_SUB:
                    HIR_BINOP_ARITHM_OPT(LLVMBuildSub, LLVMBuildSub, LLVMBuildFSub);
                case HIR_BINOP_ARITHM_MUL:
                    HIR_BINOP_ARITHM_OPT(LLVMBuildMul, LLVMBuildMul, LLVMBuildFMul);
                case HIR_BINOP_ARITHM_DIV:
                    HIR_BINOP_ARITHM_OPT(LLVMBuildSDiv, LLVMBuildUDiv, LLVMBuildFDiv);
            }
            UNREACHABLE;
        case HIR_BINOP_ORDER:
            #define HIR_BINOP_ORDER_OPT(INT, UINT, FLOAT, ...) \
                switch (binop->arithm.number.kind) { \
                    case HIR_NUMBER_INT: \
                        return LLVMBuildICmp(module->builder, INT, ls, rs, ""); \
                    case HIR_NUMBER_UINT: \
                        return LLVMBuildICmp(module->builder, UINT, ls, rs, ""); \
                    case HIR_NUMBER_FLOAT: \
                        return LLVMBuildFCmp(module->builder, FLOAT, ls, rs, ""); \
                } \
                UNREACHABLE;
            switch (binop->order.kind) {
                case HIR_BINOP_ORDER_LT:
                    HIR_BINOP_ORDER_OPT(LLVMIntSLT, LLVMIntULT, LLVMRealOLT);
                case HIR_BINOP_ORDER_GT:
                    HIR_BINOP_ORDER_OPT(LLVMIntSGT, LLVMIntUGT, LLVMRealOGT);
                case HIR_BINOP_ORDER_LE:
                    HIR_BINOP_ORDER_OPT(LLVMIntSLE, LLVMIntULE, LLVMRealOLE);
                case HIR_BINOP_ORDER_GE:
                    HIR_BINOP_ORDER_OPT(LLVMIntSGE, LLVMIntUGE, LLVMRealOGE);
            }
            UNREACHABLE;
        case HIR_BINOP_BOOL:
            switch (binop->boolean.kind) {
                case HIR_BINOP_BOOL_OR: return LLVMBuildOr(module->builder, ls, rs, "");
                case HIR_BINOP_BOOL_AND: return LLVMBuildAnd(module->builder, ls, rs, "");
            }
            UNREACHABLE;
        case HIR_BINOP_INT:
            switch (binop->integer.kind) {
                case HIR_BINOP_INT_MOD: return LLVMBuildURem(module->builder, ls, rs, "");
                case HIR_BINOP_INT_BITOR: return LLVMBuildOr(module->builder, ls, rs, "");
                case HIR_BINOP_INT_BITAND: return LLVMBuildAnd(module->builder, ls, rs, "");
                case HIR_BINOP_INT_SHR: return LLVMBuildLShr(module->builder, ls, rs, "");
                case HIR_BINOP_INT_SHL: return LLVMBuildShl(module->builder, ls, rs, "");
            }
            UNREACHABLE;
    }
    UNREACHABLE;
}

LLVMValueRef llvm_emit_const(LlvmModule *module, const HirConst *constant) {
    switch (constant->kind) {
        case HIR_CONST_INT:
            return LLVMConstInt(llvm_runtime_type(module, constant->type), constant->integer, false);
        case HIR_CONST_BOOL:
            return LLVMConstInt(LLVMInt1TypeInContext(module->context), constant->boolean, false);
        case HIR_CONST_REAL:
            return LLVMConstReal(llvm_runtime_type(module, constant->type), constant->real);
        case HIR_CONST_UNDEFINED:
            return LLVMGetUndef(llvm_runtime_type(module, constant->type));
        case HIR_CONST_STRUCT: {
            LLVMValueRef *fields = alloca(sizeof(LLVMValueRef) * vec_len(constant->struct_fields));
            for (size_t i = 0; i < vec_len(constant->struct_fields); i++) {
                fields[i] = llvm_emit_const(module, constant->struct_fields[i]);
            }
            return LLVMConstStruct(fields, vec_len(constant->struct_fields), false);
        }
        case HIR_CONST_FUNC: return module->decls[constant->func_decl];
        case HIR_CONST_GEN_FUNC:
            return module->gen_scopes[constant->gen_func.scope].funcs[constant->gen_func.func][constant->gen_func.usage];
    }
    UNREACHABLE;
}

static LlvmEmitStepRes llvm_emit_expr_step(
    LlvmModule *module,
    LlvmEmitStepRes *results,
    HirExprStep *steps, size_t step_id,
    LlvmEmitExprCtx *ctx UNUSED
) {
    HirExprStep *step = &steps[step_id];
    switch (step->kind) {
        case HIR_EXPR_STEP_CONST: return llvm_emit_step_res_new(llvm_emit_const(module, &step->constant), true);
        case HIR_EXPR_STEP_GET_DECL: {
            return llvm_emit_step_res_new(module->decls[step->decl_id], 
                    hir_get_decl_mutability(module->hir, step->decl_id));
        }
        case HIR_EXPR_STEP_CALL: {
            LLVMValueRef *args = alloca(sizeof(LLVMValueRef) * vec_len(step->call.args));
            for (size_t i = 0; i < vec_len(step->call.args); i++) {
                args[i] = llvm_get_res_value(module, &results[step->call.args[i]]);
            }
            return llvm_emit_step_res_new(LLVMBuildCall2(
                module->builder,
                llvm_function_type(module, steps[step->call.callable].type),
                llvm_get_res_value(module, &results[step->call.callable]),
                args, vec_len(step->call.args),
                ""
            ), true);
        }
        case HIR_EXPR_STEP_STRING: {
            LLVMValueRef str_global = LLVMAddGlobal(module->module, 
                LLVMArrayType(LLVMInt8TypeInContext(module->context), step->string.length), "");
            LLVMSetInitializer(str_global, LLVMConstString(step->string.value, step->string.length, true));
            LLVMValueRef str_ptr = LLVMBuildBitCast(module->builder, str_global,
                LLVMPointerTypeInContext(module->context, 0), "");
            return llvm_emit_step_res_new(str_ptr, true);
        }
        case HIR_EXPR_STEP_GET_LOCAL: {
            const HirFuncInfo *info = hir_get_func_info(module->hir, module->func.id);
            HirFuncLocal *local = &info->locals[step->local_id];
            return llvm_emit_step_res_new(module->func.locals[step->local_id], local->mutability == HIR_IMMUTABLE);
        }
        case HIR_EXPR_STEP_BINOP:
            return llvm_emit_step_res_new(llvm_emit_expr_binop(module, results, &step->binop), true);
        case HIR_EXPR_STEP_BOOL_SKIP: {
            LLVMValueRef check = llvm_get_res_value(module, &results[step->bool_skip.condition]);
            if (!step->bool_skip.expect) {
                check = LLVMBuildNot(module->builder, check, "");
            }
            LLVMBasicBlockRef cont = LLVMAppendBasicBlock(module->func.value, "");
            LLVMBuildCondBr(module->builder, check, llvm_emit_expr_ctx_end(module, ctx), cont);
            llvm_emit_expr_ctx_append(module, ctx, LLVMConstInt(LLVMInt1TypeInContext(module->context),
                step->bool_skip.result, false));
            llvm_module_set_block(module, cont);
            return llvm_emit_step_res_new(NULL, false);
        }
        case HIR_EXPR_STEP_TAKE_REF: {
            assert(!results[step->ref_step].loaded);
            return llvm_emit_step_res_new(results[step->ref_step].value, true);
        }
        case HIR_EXPR_STEP_DEREF:
            return llvm_emit_step_res_new(llvm_get_res_value(module, &results[step->deref_step]), false);
        case HIR_EXPR_STEP_STRUCT_FIELD: {
            LlvmEmitStepRes *res = &results[step->struct_field.step];
            if (res->loaded) {
                return llvm_emit_step_res_new(LLVMBuildExtractValue(module->builder, res->value,
                    step->struct_field.idx, ""), true);
            } else {
                LLVMTypeRef type = llvm_runtime_type(module, steps[step->struct_field.step].type);
                LLVMValueRef indices[] = {
                    LLVMConstInt(LLVMInt32Type(), 0, false),
                    LLVMConstInt(LLVMInt32Type(), step->struct_field.idx, false)
                };
                return llvm_emit_step_res_new(LLVMBuildGEP2(module->builder, type, res->value, indices, 2, ""), false);
            }
        }
        case HIR_EXPR_STEP_BUILD_STRUCT: {
            HirType *type = hir_resolve_simple_type(module->hir, step->build_struct.type);
            assert(type->kind == HIR_TYPE_STRUCT);
            LLVMTypeRef llvm_type = llvm_runtime_type(module, step->build_struct.type);
            LLVMValueRef result = llvm_alloca(module, llvm_type);
            for (size_t i = 0; i < vec_len(step->build_struct.fields); i++) {
                LLVMValueRef value = llvm_get_res_value(module, &results[step->build_struct.fields[i]]);
                LLVMValueRef indices[] = {
                    LLVMConstInt(LLVMInt32TypeInContext(module->context), 0, false),
                    LLVMConstInt(LLVMInt32TypeInContext(module->context), i, false),
                };
                LLVMValueRef field = LLVMBuildGEP2(module->builder, llvm_type, result, indices, 2, "");
                LLVMBuildStore(module->builder, value, field);
            }
            return llvm_emit_step_res_new(LLVMBuildLoad2(module->builder, llvm_type, result, ""), true);
        }
        case HIR_EXPR_STEP_CAST_INT: {
            LLVMValueRef what = llvm_get_res_value(module, &results[step->cast_int.step_id]);
            LLVMTypeRef type = llvm_runtime_type(module, step->cast_int.dest);
            HirTypeInt *source = &hir_resolve_simple_type(module->hir, step->cast_int.source)->integer;
            HirTypeInt *dest = &hir_resolve_simple_type(module->hir, step->cast_int.dest)->integer;
            if (source->size > dest->size) {
                return llvm_emit_step_res_new(LLVMBuildTrunc(module->builder, what, type, ""), true);
            } else {
                return llvm_emit_step_res_new(
                    (source->is_signed && dest->is_signed) ?
                        LLVMBuildSExt(module->builder, what, type, "") :
                        LLVMBuildZExt(module->builder, what, type, ""), true);
            }
        }
        case HIR_EXPR_STEP_NOT:
            return llvm_emit_step_res_new(LLVMBuildNot(module->builder, llvm_get_res_value(module,
                &results[step->not_step]), ""), true);
        case HIR_EXPR_STEP_NEG:
            switch (step->neg.info.kind) {
                case HIR_NUMBER_INT:
                case HIR_NUMBER_UINT: return llvm_emit_step_res_new(LLVMBuildNeg(module->builder,
                    llvm_get_res_value(module, &results[step->neg.step]), ""), true);
                case HIR_NUMBER_FLOAT: return llvm_emit_step_res_new(LLVMBuildFNeg(module->builder,
                    llvm_get_res_value(module, &results[step->neg.step]), ""), true);
            }
            UNREACHABLE;
        case HIR_EXPR_STEP_SIZEOF: {
            LLVMValueRef indices[] = {
                LLVMConstInt(LLVMInt32TypeInContext(module->context), 1, false),
            };
            LLVMValueRef pointer = LLVMBuildGEP2(module->builder, llvm_runtime_type(module, step->size.of),
                LLVMConstPointerNull(LLVMPointerTypeInContext(module->context, 0)), indices, 1, "");
            return llvm_emit_step_res_new(
                LLVMBuildPtrToInt(module->builder, pointer, llvm_runtime_type(module, step->size.type), ""), true);
        }
        case HIR_EXPR_STEP_BUILD_ARRAY: {
            LLVMTypeRef llvm_type = llvm_runtime_type(module, step->type);
            LLVMValueRef result = llvm_alloca(module, llvm_type);
            for (size_t i = 0; i < vec_len(step->build_array.elements); i++) {
                LLVMValueRef value = llvm_get_res_value(module, &results[step->build_array.elements[i]]);
                LLVMValueRef indices[] = {
                    LLVMConstInt(LLVMInt32TypeInContext(module->context), 0, false),
                    LLVMConstInt(LLVMInt32TypeInContext(module->context), i, false),
                };
                LLVMValueRef field = LLVMBuildGEP2(module->builder, llvm_type, result, indices, 2, "");
                LLVMBuildStore(module->builder, value, field);
            }
            return llvm_emit_step_res_new(LLVMBuildLoad2(module->builder, llvm_type, result, ""), true);
        }
        case HIR_EXPR_STEP_IDX_ARRAY: {
            LlvmEmitStepRes *res = &results[step->idx_array.value];
            if (res->loaded) {
                return llvm_emit_step_res_new(LLVMBuildExtractValue(module->builder, res->value,
                    step->idx_array.idx, ""), true);
            } else {
                LLVMTypeRef type = llvm_runtime_type(module, steps[step->idx_array.value].type);
                LLVMValueRef indices[] = {
                    LLVMConstInt(LLVMInt32Type(), 0, false),
                    llvm_get_res_value(module, &results[step->idx_array.idx])
                };
                return llvm_emit_step_res_new(LLVMBuildGEP2(module->builder, type, res->value, indices, 2, ""), false);
            }
        }
        case HIR_EXPR_STEP_IDX_POINTER: {
            LLVMTypeRef type = llvm_runtime_type(module, step->type);
            LLVMValueRef pointer = llvm_get_res_value(module, &results[step->idx_pointer.value]);
            LLVMValueRef idx = llvm_get_res_value(module, &results[step->idx_pointer.idx]);
            return llvm_emit_step_res_new(LLVMBuildGEP2(module->builder, type, pointer, &idx, 1, ""), false);
        }
        case HIR_EXPR_STEP_CAST_PTR: {
            LLVMTypeRef type = llvm_runtime_type(module, step->type);
            LLVMValueRef pointer = llvm_get_res_value(module, &results[step->cast_ptr.step_id]);
            return llvm_emit_step_res_new(LLVMBuildBitCast(module->builder, pointer, type, ""), true);
        }
        case HIR_EXPR_STEP_INT_TO_PTR: {
            LLVMTypeRef type = llvm_runtime_type(module, step->int_to_ptr.type);
            LLVMValueRef value = llvm_get_res_value(module, &results[step->int_to_ptr.step_id]);
            return llvm_emit_step_res_new(LLVMBuildIntToPtr(module->builder, value, type, ""), true);
        }
        case HIR_EXPR_STEP_PTR_TO_INT: {
            LLVMTypeRef type = llvm_runtime_type(module, step->ptr_to_int.type);
            LLVMValueRef value = llvm_get_res_value(module, &results[step->ptr_to_int.step_id]);
            return llvm_emit_step_res_new(LLVMBuildBitCast(module->builder, value, type, ""), true);
        }
    }
    UNREACHABLE;
}

LLVMValueRef llvm_emit_expr(LlvmModule *module, const HirExpr *expr, bool load) {
    LlvmEmitStepRes *results = alloca(sizeof(LlvmEmitStepRes) * vec_len(expr->steps));
    LlvmEmitExprCtx ctx = llvm_emit_expr_ctx_new();
    for (size_t i = 0; i < vec_len(expr->steps); i++) {
        results[i] = llvm_emit_expr_step(module, results, expr->steps, i, &ctx);
        if (results[i].value) {
            results[i].type = llvm_runtime_type(module, expr->steps[i].type);
        }
    }
    size_t last_step = vec_len(expr->steps) - 1;
    if (!llvm_emit_expr_ctx_is_empty(&ctx)) {
        llvm_emit_expr_ctx_append(module, &ctx, llvm_get_res_value(module, &results[last_step]));
        LLVMBuildBr(module->builder, ctx.end);
        llvm_module_set_block(module, ctx.end);
        LLVMValueRef result = LLVMBuildPhi(module->builder, llvm_runtime_type(module, expr->steps[last_step].type), "");
        LLVMAddIncoming(result, ctx.values, ctx.bbs, llvm_emit_expr_ctx_len(&ctx));
        return result;
    }
    if (!load) {
        assert(!results[last_step].loaded);
        return results[last_step].value;
    }
    return llvm_get_res_value(module, &results[last_step]);
}

