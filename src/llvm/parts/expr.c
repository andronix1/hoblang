#include "../parts.h"
#include "../utils/slices.h"

bool llvm_is_signed(SemaType *type) {
	return
		type->primitive == PRIMITIVE_I8 || 
		type->primitive == PRIMITIVE_I16 || 
		type->primitive == PRIMITIVE_I32 || 
		type->primitive == PRIMITIVE_I64;
}

LLVMValueRef llvm_call(LlvmBackend *llvm, AstCall *call) {
	LLVMValueRef *params = alloca(sizeof(LLVMValueRef) * vec_len(call->args));
	for (size_t i = 0; i < vec_len(call->args); i++) {
		params[i] = llvm_expr(llvm, call->args[i]);
	}
	return LLVMBuildCall2(
		llvm->builder,
		llvm_sema_function_type(&call->callable->sema_type->func),
		LLVMBuildLoad2(
			llvm->builder,
			llvm_resolve_type(call->callable->sema_type),
			NULL, // TODO: expression
			// llvm_value(llvm, &call->callable),
			""
		),
		params, vec_len(call->args),
		""
	);
}

LLVMValueRef llvm_get_local_value_path(LlvmBackend *llvm, AstPath *path) {
	LLVMValueRef result = NULL;
	for (size_t i = 0; i < vec_len(path->segments); i++) {
		SemaPathSegment *segment = &path->segments[i].sema;
		switch (segment->type) {
			case SEMA_PATH_SEGMENT_MODULE:
			case SEMA_PATH_SEGMENT_TYPE: 
				break;
			case SEMA_PATH_SEGMENT_DECL:
				result = segment->decl->value_decl.llvm_value;
				printf("GET(llvm) %p\n", &segment->decl->value_decl);
				break;
			case SEMA_PATH_SEGMENT_STRUCT_MEMBER: {
				LLVMValueRef indices[2] = {
					LLVMConstInt(LLVMInt32Type(), 0, false),
					LLVMConstInt(LLVMInt32Type(), segment->struct_member.member_id, false)
				};
				result = LLVMBuildGEP2(
					llvm->builder,
					llvm_resolve_type(segment->struct_member.struct_type),
					result,
					indices, 2,
					"struct_member"
				);
				break;
			}
			case SEMA_PATH_SEGMENT_SLICE_LENGTH:
				result = llvm_slice_len(
					llvm,
					llvm_resolve_type(segment->slice_type),
					result
				);
				break;
			case SEMA_PATH_SEGMENT_SLICE_PTR:
				result = llvm_slice_ptr(
					llvm,
					llvm_resolve_type(segment->slice_type),
					result
				);
				break;
		}
	}
	assert(result, "path `{ast::path}` was not resolved!", path);
	return result;
}

LLVMValueRef llvm_expr(LlvmBackend *llvm, AstExpr *expr) {
	switch (expr->type) {
		case AST_EXPR_GET_INNER_PATH:
			assert(0, "NIY");
		case AST_EXPR_GET_LOCAL_PATH: return llvm_get_local_value_path(llvm, &expr->get_local.path);
		case AST_EXPR_NOT: return LLVMBuildNot(llvm->builder, llvm_expr(llvm, expr->not_expr), "");
		// case AST_EXPR_VALUE: return LLVMBuildLoad2(llvm->builder, llvm_resolve_type(expr->sema_type), llvm_value(llvm, &expr->value), "");
		case AST_EXPR_REF: return llvm_expr(llvm, expr->ref_expr);
		case AST_EXPR_INTEGER: return LLVMConstInt(llvm_resolve_type(expr->sema_type), expr->integer, false);
		case AST_EXPR_BOOL: return LLVMConstInt(LLVMInt1Type(), expr->boolean, false);
		case AST_EXPR_CHAR: return LLVMConstInt(LLVMInt8Type(), expr->character, false);
		case AST_EXPR_STR: return llvm_slice_from_str(llvm, &expr->str);
		case AST_EXPR_AS: {
			LLVMTypeRef to_type = llvm_resolve_type(expr->as.type.sema);
			LLVMValueRef value = llvm_expr(llvm, expr->as.expr);
			switch (expr->as.conv_type) {
				case SEMA_AS_CONV_EXTEND: return LLVMBuildZExt(llvm->builder, value, to_type, "");
				case SEMA_AS_CONV_TRUNC: return LLVMBuildTrunc(llvm->builder, value, to_type, "");
				case SEMA_AS_CONV_BITCAST: return LLVMBuildBitCast(llvm->builder, value, to_type, "");
				case SEMA_AS_CONV_PTR_TO_INT: return LLVMBuildPtrToInt(llvm->builder, value, to_type, "");
				case SEMA_AS_CONV_INT_TO_PTR: return LLVMBuildIntToPtr(llvm->builder, value, to_type, "");
				case SEMA_AS_CONV_ARR_PTR_TO_SLICE: return llvm_slice_from_array_ptr(
					llvm,
					llvm_resolve_type(expr->as.expr->sema_type->array.of),
					value,
					expr->as.expr->sema_type->array.length
				);
				case SEMA_AS_CONV_SLICE_TO_PTR: return LLVMBuildExtractValue( llvm->builder, value, 1, "");
			}
			break;
		}
		case AST_EXPR_UNARY: {
			switch (expr->unary.type) {
				case AST_UNARY_MINUS: return LLVMBuildNeg(llvm->builder, llvm_expr(llvm, expr->unary.expr), "");
				case AST_UNARY_BITNOT: return LLVMBuildNot(llvm->builder, llvm_expr(llvm, expr->unary.expr), "");
			}
			assert(0, "invalid unary {int}", expr->unary.type);
			return NULL;
		}
		case AST_EXPR_BINOP: {
			LLVMValueRef right = llvm_expr(llvm, expr->binop.right);
			LLVMValueRef left = llvm_expr(llvm, expr->binop.left);
			switch (expr->binop.type) {
				case AST_BINOP_ADD: return LLVMBuildAdd(llvm->builder, left, right, "");
				case AST_BINOP_XOR: return LLVMBuildXor(llvm->builder, left, right, "");
				case AST_BINOP_BITAND: return LLVMBuildAnd(llvm->builder, left, right, "");
				case AST_BINOP_BITOR: return LLVMBuildOr(llvm->builder, left, right, "");
				case AST_BINOP_SHR: return LLVMBuildLShr(llvm->builder, left, right, "");
				case AST_BINOP_SHL: return LLVMBuildShl(llvm->builder, left, right, "");
				case AST_BINOP_SUB: return LLVMBuildSub(llvm->builder, left, right, "");
				case AST_BINOP_MUL: return LLVMBuildMul(llvm->builder, left, right, "");
				case AST_BINOP_DIV: return LLVMBuildSDiv(llvm->builder, left, right, "");
				case AST_BINOP_EQ: return LLVMBuildICmp(llvm->builder, LLVMIntEQ, left, right, "");
				case AST_BINOP_NEQ: return LLVMBuildICmp(llvm->builder, LLVMIntNE, left, right, "");
				case AST_BINOP_GT: return LLVMBuildICmp(llvm->builder, LLVMIntSGT, left, right, "");
				case AST_BINOP_GE: return LLVMBuildICmp(llvm->builder, LLVMIntSGE, left, right, "");
				case AST_BINOP_LT: return LLVMBuildICmp(llvm->builder, LLVMIntSLT, left, right, "");
				case AST_BINOP_LE: return LLVMBuildICmp(llvm->builder, LLVMIntSLE, left, right, "");
				case AST_BINOP_AND: return LLVMBuildAnd(llvm->builder, left, right, "");
				case AST_BINOP_OR: return LLVMBuildOr(llvm->builder, left, right, "");
			}
			assert(0, "invalid binop {int}", expr->binop.type);
			return NULL;
		}
		case AST_EXPR_CALL: return llvm_call(llvm, &expr->call);	
		case AST_EXPR_ARRAY: {
			LLVMTypeRef of = llvm_resolve_type(expr->sema_type->array.of);
			LLVMTypeRef type = LLVMArrayType(of, vec_len(expr->array));
			LLVMValueRef array = LLVMBuildAlloca(llvm->builder, type, "new_arr");
			for (size_t i = 0; i < vec_len(expr->array); i++) {
				LLVMValueRef indices[1] = { LLVMConstInt(LLVMInt32Type(), i, false) };
				LLVMBuildStore(
					llvm->builder,
					llvm_expr(llvm, expr->array[i]),
					LLVMBuildGEP2(llvm->builder, of, array, indices, 1, "arri")
				);
			}
			
			return LLVMBuildLoad2(llvm->builder, type, array, "");
		}
	}
	assert(0, "invalid expr {int}", expr->type);
	return NULL;
}
