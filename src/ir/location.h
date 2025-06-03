#pragma once

#include "core/file_content.h"
#include "core/slice.h"

typedef struct {
    FileContent *content;
    Slice slice;
} IrLocation;

static inline IrLocation ir_loc_new(FileContent *content, Slice slice) {
    IrLocation loc = {
        .content = content,
        .slice = slice
    };
    return loc;
}
