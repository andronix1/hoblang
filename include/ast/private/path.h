#pragma once

#include "core/slice.h"
#include "sema/value.h"
#include "sema/module/decls.h"
#include "sema/module.h"
#include "sema/type.h"
#include "sema/value.h"

typedef struct AstDeclPath {
	Slice *segments;

	union {
		SemaModule *module;
		SemaScopeDecl *decl;
    	SemaType *type;
	};
} AstDeclPath;

typedef enum {
	AST_INNER_PATH_SEG_IDENT,
	AST_INNER_PATH_SEG_DEREF,
} AstInnerPathSegmentType;

typedef enum {
	SEMA_INNER_PATH_DEREF,
    SEMA_INNER_PATH_SLICE_LEN,
    SEMA_INNER_PATH_SLICE_RAW,
    SEMA_INNER_PATH_STRUCT_MEMBER,
} SemaInnerPathType;

typedef struct {
    size_t idx;
    SemaType *of;
} SemaInnerPathStructMember;

typedef struct {
    SemaInnerPathType type;
    SemaValue *value;
    union {
        SemaType *deref_type;
        SemaType *slice_type;
        SemaInnerPathStructMember struct_member;
    };
} SemaInnerPath;

typedef struct {
	AstInnerPathSegmentType type;
	union {
		Slice ident;
	};
    SemaInnerPath sema;
} AstInnerPathSegment;

typedef struct AstInnerPath {
	AstInnerPathSegment *segments;
} AstInnerPath;

typedef enum {
    AST_PATH_SEGMENT_IDENT
} AstPathSegmentType;

typedef struct AstPath {
	AstDeclPath decl_path;
	AstInnerPath inner_path;
} AstPath;
