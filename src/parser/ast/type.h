#pragma once

#include "core/slice.h"
#include "path.h"

struct _SemaType;

typedef enum {
	AST_TYPE_PATH,
	AST_TYPE_STRUCT,
	AST_TYPE_FUNCTION,
	AST_TYPE_SLICE,
	AST_TYPE_ARRAY,
	AST_TYPE_POINTER
} AstTypeKind;

typedef struct {
	struct _AstType *args;
	struct _AstType *returns;
} AstFunctionType;

typedef struct {
	Slice name;
	struct _AstType *type;
} AstStructMember;

typedef struct {
	AstStructMember *members;
} AstStructType;

typedef struct {
	struct _AstType *of;
	struct _AstExpr *length;
} AstArrayType;

typedef struct _AstType {
	AstTypeKind type;
	struct _SemaType *sema;
	union {
		AstPath path;
		AstStructType struct_type;
		AstFunctionType func;
		struct _AstType *ptr_to;
		struct _AstType *slice_of;
		AstArrayType array;
	};
} AstType;
