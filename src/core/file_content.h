#pragma once

#include "core/path.h"
#include "core/slice.h"
#include <stdarg.h>

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

typedef struct {
    size_t start, end;
    Slice content;
} FilePosLinesHandle;

typedef struct {
    FilePosLinesHandle handle;
    Slice slice;
} FileInLinesView;

FileContent *file_content_new_in_memory(const char *data);
FileContent *file_content_read(const Path path);
FilePos file_content_locate_pos(const FileContent *content, size_t pos);
FileLoc file_content_locate(const FileContent *content, Slice slice);
FilePosLinesHandle file_content_get_lines(const FileContent *content, size_t start, size_t end);
FileInLinesView file_content_get_in_lines_view(const FileContent *content, Slice slice);
void file_content_free(FileContent *content);

void file_pos_print(va_list list);
void file_in_lines_view_print(va_list list);
