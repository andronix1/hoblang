#pragma once

#include <llvm-c/Core.h>
#include <stdarg.h>
#include "core/slice.h"
#include "core/vec.h"
#include "parser/ast.h"

typedef struct {
	struct _SemaType **args;
	struct _SemaType *returning;
} SemaFunction;

typedef enum {
	PRIMITIVE_I8, PRIMITIVE_I16, PRIMITIVE_I32, PRIMITIVE_I64,
	PRIMITIVE_U8, PRIMITIVE_U16, PRIMITIVE_U32, PRIMITIVE_U64,
	PRIMITIVE_BOOL,
	PRIMITIVE_VOID
} Primitive;

typedef enum {
	SEMA_TYPE_PRIMITIVE,
	SEMA_TYPE_POINTER,
	SEMA_TYPE_FUNCTION,
	SEMA_TYPE_STRUCT,
	SEMA_TYPE_SLICE,
	SEMA_TYPE_ARRAY,
} SemaTypeKind;

typedef struct {
	struct _SemaType *of;
	size_t length;
} SemaArrayType;

typedef struct _SemaType {
	SemaTypeKind type;
	union {
		LLVMTypeRef llvm_type;
	};
	union {
		Primitive primitive;
		SemaFunction func;
		struct _SemaType *ptr_to;
		struct _SemaType *slice_of;
		AstStructType *struct_type;
		SemaArrayType array;
	};
} SemaType;

extern SemaType primitives[];

SemaType *sema_type_new_array(size_t length, SemaType *of);
SemaType *sema_type_new_slice(SemaType *of);
SemaType *sema_type_new_func(SemaType *returning, SemaType **args);
SemaType *sema_type_new_pointer(SemaType *to);

bool sema_types_equals(SemaType *type, SemaType *other);
void print_sema_type(FILE* stream, va_list list);
