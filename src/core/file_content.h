#pragma once

#include "core/path.h"
#include "core/slice.h"

typedef struct {
    const Path path;
    const Slice content;
} FileContent;

FileContent *file_content_read(const Path path);
void file_content_free(FileContent *content);
