#pragma once

#include "sema/module/api/decl_handle.h"

typedef enum {
    SEMA_PATH_SEGMENT_IGNORE,
    SEMA_PATH_SEGMENT_DECL,
} SemaPathSegmentExtKind;

typedef struct {
    SemaPathSegmentExtKind kind;

    union {
        SemaDeclHandle *decl;
    };
} SemaPathSegmentExt;
