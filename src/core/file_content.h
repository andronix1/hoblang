#pragma once

#include "core/path.h"
#include "core/slice.h"

typedef struct {
    const Path path;
    const Slice data;
    const bool allocated;
} FileContent;

typedef struct {
    size_t line, character;
} FilePos;

typedef struct {
    FilePos begin, end;
} FileLoc;

FileContent *file_content_new_in_memory(const char *data);
FileContent *file_content_read(const Path path);
FileLoc file_content_locate(FileContent *content, Slice slice);
void file_content_free(FileContent *content);
