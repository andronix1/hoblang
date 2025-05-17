#pragma once

#include "sema/module/api/decl_handle.h"
#include "sema/module/api/value.h"
#include <stddef.h>

typedef enum {
    SEMA_PATH_SEGMENT_IGNORE,
    SEMA_PATH_SEGMENT_DECL,
    SEMA_PATH_SEGMENT_STRUCT_FIELD,
    SEMA_PATH_SEGMENT_EXT_DIRECT,
    SEMA_PATH_SEGMENT_EXT_REF,
    SEMA_PATH_SEGMENT_DEREF,
} SemaPathSegmentExtKind;

typedef struct {
    SemaPathSegmentExtKind kind;
    SemaValue *from_value;

    union {
        SemaDeclHandle *decl;
        struct {
            SemaDeclHandle *handle, *decl;
        } ext;
        size_t struct_field_idx;
    };
} SemaPathSegmentExt;
