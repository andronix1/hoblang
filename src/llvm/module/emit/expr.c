#include "expr.h"
#include "core/assert.h"
#include "core/mempool.h"
#include "core/vec.h"
#include "ir/api/ir.h"
#include "ir/const.h"
#include "ir/decls.h"
#include "ir/stmt/expr.h"
#include "llvm/module/module.h"
#include "ir/ir.h"
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

static inline LLVMBasicBlockRef llvm_emit_expr_ctx_end(LlvmModule *module, LlvmEmitExprCtx *ctx) {
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

static LLVMValueRef llvm_emit_expr_binop(LlvmModule *module, LlvmEmitStepRes *results, IrBinop *binop) {
    LLVMValueRef ls = llvm_get_res_value(module, &results[binop->ls]);
    LLVMValueRef rs = llvm_get_res_value(module, &results[binop->rs]);
    switch (binop->kind) {
        case IR_BINOP_COMPARE:
            switch (binop->compare.kind) {
                case IR_COMPARE_EQ:
                    switch (binop->compare.val_kind) {
                        case IR_COMPARE_NUMBER:
                        case IR_COMPARE_BOOL:
                            return LLVMBuildICmp(module->builder, LLVMIntEQ, ls, rs, "");
                    }
                    UNREACHABLE;
                case IR_COMPARE_NE:
                    switch (binop->compare.val_kind) {
                        case IR_COMPARE_NUMBER:
                        case IR_COMPARE_BOOL:
                            return LLVMBuildICmp(module->builder, LLVMIntNE, ls, rs, "");
                    }
                    UNREACHABLE;
            }
            UNREACHABLE;
        case IR_BINOP_ARITHMETIC:
            #define IR_BINOP_ARITHM_OPT(INT, UINT, FLOAT, ...) \
                switch (binop->arithm.number.kind) { \
                    case IR_NUMBER_INT: \
                        return INT(module->builder, ls, rs, ""); \
                    case IR_NUMBER_UINT: \
                        return UINT(module->builder, ls, rs, ""); \
                    case IR_NUMBER_FLOAT: \
                        return FLOAT(module->builder, ls, rs, ""); \
                } \
                UNREACHABLE;
            switch (binop->arithm.kind) {
                case IR_BINOP_ARITHM_ADD:
                    IR_BINOP_ARITHM_OPT(LLVMBuildAdd, LLVMBuildAdd, LLVMBuildFAdd);
                case IR_BINOP_ARITHM_SUB:
                    IR_BINOP_ARITHM_OPT(LLVMBuildSub, LLVMBuildSub, LLVMBuildFSub);
                case IR_BINOP_ARITHM_MUL:
                    IR_BINOP_ARITHM_OPT(LLVMBuildMul, LLVMBuildMul, LLVMBuildFMul);
                case IR_BINOP_ARITHM_DIV:
                    IR_BINOP_ARITHM_OPT(LLVMBuildSDiv, LLVMBuildUDiv, LLVMBuildFDiv);
            }
            UNREACHABLE;
        case IR_BINOP_ORDER:
            #define IR_BINOP_ORDER_OPT(INT, UINT, FLOAT, ...) \
                switch (binop->arithm.number.kind) { \
                    case IR_NUMBER_INT: \
                        return LLVMBuildICmp(module->builder, INT, ls, rs, ""); \
                    case IR_NUMBER_UINT: \
                        return LLVMBuildICmp(module->builder, UINT, ls, rs, ""); \
                    case IR_NUMBER_FLOAT: \
                        return LLVMBuildFCmp(module->builder, FLOAT, ls, rs, ""); \
                } \
                UNREACHABLE;
            switch (binop->order.kind) {
                case IR_BINOP_ORDER_LT:
                    IR_BINOP_ORDER_OPT(LLVMIntSLT, LLVMIntULT, LLVMRealOLT);
                case IR_BINOP_ORDER_GT:
                    IR_BINOP_ORDER_OPT(LLVMIntSGT, LLVMIntUGT, LLVMRealOGT);
                case IR_BINOP_ORDER_LE:
                    IR_BINOP_ORDER_OPT(LLVMIntSLE, LLVMIntULE, LLVMRealOLE);
                case IR_BINOP_ORDER_GE:
                    IR_BINOP_ORDER_OPT(LLVMIntSGE, LLVMIntUGE, LLVMRealOGE);
            }
            UNREACHABLE;
        case IR_BINOP_BOOL:
            switch (binop->boolean.kind) {
                case IR_BINOP_BOOL_OR: return LLVMBuildOr(module->builder, ls, rs, "");
                case IR_BINOP_BOOL_AND: return LLVMBuildAnd(module->builder, ls, rs, "");
            }
            UNREACHABLE;
        case IR_BINOP_INT:
            switch (binop->integer.kind) {
                case IR_BINOP_INT_MOD: return LLVMBuildURem(module->builder, ls, rs, "");
            }
            UNREACHABLE;
    }
    UNREACHABLE;
}

LLVMValueRef llvm_emit_const(LlvmModule *module, IrConst *constant) {
    switch (constant->kind) {
        case IR_CONST_BOOL:
            return LLVMConstInt(LLVMInt1Type(), constant->boolean, false);
        case IR_CONST_INT:
            return LLVMConstInt(
                module->types[constant->type],
                constant->integer,
                ir_type_int_is_signed(module->ir, constant->type)
            );
        case IR_CONST_REAL: TODO;
        case IR_CONST_STRUCT: {
            LLVMValueRef *fields = alloca(sizeof(LLVMValueRef) * vec_len(constant->struct_fields));
            for (size_t i = 0; i < vec_len(constant->struct_fields); i++) {
                fields[i] = llvm_emit_const(module, constant->struct_fields[i]);
            }
            return LLVMConstStruct(fields, vec_len(constant->struct_fields), false);
        }
        case IR_CONST_DECL_PTR: return module->decls[constant->decl];
    }
    UNREACHABLE;
}

static LlvmEmitStepRes llvm_emit_expr_step(
    LlvmModule *module,
    LlvmEmitStepRes *results,
    IrExprStep *steps, size_t step_id,
    LlvmEmitExprCtx *ctx
) {
    IrExprStep *step = &steps[step_id];
    switch (step->kind) {
        case IR_EXPR_STEP_CONST: return llvm_emit_step_res_new(llvm_emit_const(module, step->constant), true);
        case IR_EXPR_STEP_STRING: {
            LLVMValueRef str_global = LLVMAddGlobal(module->module, LLVMArrayType(LLVMInt8Type(), step->string.length),
                "");
            LLVMSetInitializer(str_global, LLVMConstString(step->string.value, step->string.length, true));
            LLVMValueRef str_ptr = LLVMBuildBitCast(module->builder, str_global, LLVMPointerType(LLVMInt8Type(), 0), "");
            return llvm_emit_step_res_new(str_ptr, true);
        }
        case IR_EXPR_STEP_CALL: {
            LLVMValueRef *args = alloca(sizeof(LLVMValueRef) * vec_len(step->call.args));
            for (size_t i = 0; i < vec_len(step->call.args); i++) {
                args[i] = llvm_get_res_value(module, &results[step->call.args[i]]);
            }
            return llvm_emit_step_res_new(LLVMBuildCall2(
                module->builder,
                module->types[steps[step->call.callable].type],
                llvm_get_res_value(module, &results[step->call.callable]),
                args, vec_len(step->call.args),
                ""
            ), true);
        }
        case IR_EXPR_STEP_GET_DECL: {
            IrDecl *decl = &module->ir->decls[step->decl_id];
            return llvm_emit_step_res_new(module->decls[step->decl_id], decl->mutability == IR_IMMUTABLE);
        }
        case IR_EXPR_STEP_GET_LOCAL: {
            IrFuncLocal *local = &module->ir->funcs[module->func.id].locals[step->local_id];
            return llvm_emit_step_res_new(module->func.locals[step->local_id], local->mutability == IR_IMMUTABLE);
        }
        case IR_EXPR_STEP_BINOP:
            return llvm_emit_step_res_new(llvm_emit_expr_binop(module, results, &step->binop), true);
        case IR_EXPR_STEP_BOOL_SKIP: {
            LLVMValueRef check = llvm_get_res_value(module, &results[step->bool_skip.condition]);
            if (!step->bool_skip.expect) {
                check = LLVMBuildNot(module->builder, check, "");
            }
            LLVMBasicBlockRef cont = LLVMAppendBasicBlock(module->func.value, "");
            LLVMBuildCondBr(module->builder, check, llvm_emit_expr_ctx_end(module, ctx), cont);
            llvm_emit_expr_ctx_append(module, ctx, LLVMConstInt(LLVMInt1Type(),
                step->bool_skip.result, false));
            LLVMPositionBuilderAtEnd(module->builder, cont);
            return llvm_emit_step_res_new(NULL, false);
        }
        case IR_EXPR_STEP_TAKE_REF: {
            assert(!results[step->ref_step].loaded);
            return llvm_emit_step_res_new(results[step->ref_step].value, true);
        }
        case IR_EXPR_STEP_DEREF:
            return llvm_emit_step_res_new(llvm_get_res_value(module, &results[step->deref_step]), false);
        case IR_EXPR_STEP_STRUCT_FIELD: {
            LlvmEmitStepRes *res = &results[step->struct_field.step];
            if (res->loaded) {
                return llvm_emit_step_res_new(LLVMBuildExtractValue(module->builder, res->value,
                    step->struct_field.idx, ""), false);
            } else {
                LLVMTypeRef type = module->types[steps[step->struct_field.step].type];
                LLVMValueRef indices[] = {
                    LLVMConstInt(LLVMInt32Type(), 0, false),
                    LLVMConstInt(LLVMInt32Type(), step->struct_field.idx, false)
                };
                return llvm_emit_step_res_new(LLVMBuildGEP2(module->builder, type, res->value, indices, 2, ""), false);
            }
        }
        case IR_EXPR_STEP_BUILD_STRUCT: {
            IrType *type = &module->ir->types[ir_type_record_resolve_simple(module->ir, step->build_struct.type)].simple;
            assert(type->kind == IR_TYPE_STRUCT);
            LLVMTypeRef llvm_type = module->types[step->build_struct.type];
            LLVMValueRef result = LLVMBuildLoad2(module->builder, llvm_type, llvm_alloca(module, llvm_type), "");
            for (size_t i = 0; i < vec_len(step->build_struct.fields); i++) {
                result = LLVMBuildInsertValue(module->builder, result, llvm_get_res_value(module,
                    &results[step->build_struct.fields[i]]), i, "");
            }
            return llvm_emit_step_res_new(result, true);
        }
        case IR_EXPR_STEP_CAST_INT: {
            LLVMValueRef what = llvm_get_res_value(module, &results[step->cast_int.step_id]);
            LLVMTypeRef type = module->types[step->cast_int.dest];
            IrTypeInt *source = &module->ir->types[ir_type_record_resolve_simple(module->ir, step->cast_int.source)].simple.integer;
            IrTypeInt *dest = &module->ir->types[ir_type_record_resolve_simple(module->ir, step->cast_int.dest)].simple.integer;
            if (source->size > dest->size) {
                return llvm_emit_step_res_new(LLVMBuildTrunc(module->builder, what, type, ""), true);
            } else {
                return llvm_emit_step_res_new(
                    (source->is_signed && dest->is_signed) ?
                        LLVMBuildSExt(module->builder, what, type, "") :
                        LLVMBuildZExt(module->builder, what, type, ""), true);
            }
        }
        case IR_EXPR_STEP_NOT:
            return llvm_emit_step_res_new(LLVMBuildNot(module->builder, llvm_get_res_value(module,
                &results[step->not_step]), ""), true);
    }
    UNREACHABLE;
}

LLVMValueRef llvm_emit_expr(LlvmModule *module, IrExpr *expr, bool load) {
    LlvmEmitStepRes *results = alloca(sizeof(LlvmEmitStepRes) * vec_len(expr->steps));
    LlvmEmitExprCtx ctx = llvm_emit_expr_ctx_new();
    for (size_t i = 0; i < vec_len(expr->steps); i++) {
        results[i] = llvm_emit_expr_step(module, results, expr->steps, i, &ctx);
        if (results[i].value) {
            results[i].type = module->types[expr->steps[i].type];
        }
    }
    size_t last_step = vec_len(expr->steps) - 1;
    if (!llvm_emit_expr_ctx_is_empty(&ctx)) {
        llvm_emit_expr_ctx_append(module, &ctx, llvm_get_res_value(module, &results[last_step]));
        LLVMBuildBr(module->builder, ctx.end);
        LLVMPositionBuilderAtEnd(module->builder, ctx.end);
        LLVMValueRef result = LLVMBuildPhi(module->builder, module->types[expr->steps[last_step].type], "");
        LLVMAddIncoming(result, ctx.values, ctx.bbs, llvm_emit_expr_ctx_len(&ctx));
        return result;
    }
    if (!load) {
        assert(!results[last_step].loaded);
        return results[last_step].value;
    }
    return llvm_get_res_value(module, &results[last_step]);
}

