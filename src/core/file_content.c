#include "file_content.h"
#include "core/ansi.h"
#include "core/log.h"
#include "core/slice.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static inline FileContent *file_content_new(const Path path, Slice data, bool allocated) {
    FileContent *result = malloc(sizeof(FileContent));
    *(Path*)&result->path = path;
    *(Slice*)&result->data = data;
    *(bool*)&result->allocated = allocated;
    return result;
}

FileContent *file_content_new_in_memory(const char *data) {
    return file_content_new("<memory>", slice_from_cstr(data), false);
}

FileContent *file_content_read(const Path path) {
    FILE *file = fopen(path, "r");
    if (!file) {
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *content = malloc(size);
    int readed_bytes = fread(content, 1, size, file);
    fclose(file);
    if (readed_bytes < 0) {
        return NULL;
    }

    return file_content_new(path, slice_new(content, readed_bytes), true);
}

inline static void file_pos_putc(FilePos *pos, char c) {
    if (c == '\n') {
        pos->line++;
        pos->character = 0;
    } else {
        pos->character++;
    }
}

FilePos file_content_locate_pos(const FileContent *content, size_t pos) {
    FilePos result = { .line = 1, .character = 0 };
    for (size_t i = 0; i <= pos; i++) {
        file_pos_putc(&result, content->data.value[i]);
    }
    return result;
}

FileLoc file_content_locate(const FileContent *content, Slice slice) {
    assert(slice.value >= content->data.value);
    size_t position = slice.value - content->data.value;
    assert(position <= content->data.length);
    FileLoc result = {
        .begin = { .line = 1, .character = 0 }
    };
    for (size_t i = 0; i <= position; i++) {
        file_pos_putc(&result.begin, content->data.value[i]);
    }
    result.end = result.begin;
    for (size_t i = 1; i < slice.length; i++) {
        file_pos_putc(&result.end, slice.value[i]);
    }
    return result;
}

static inline FilePosLinesHandle file_pos_lines_handle(Slice content, size_t start, size_t end) {
    FilePosLinesHandle handle = {
        .content = content,
        .start = start,
        .end = end,
    };
    return handle;
}

FilePosLinesHandle file_content_get_lines(const FileContent *content, size_t start, size_t end) {
    assert(start <= end);
    FilePos position = { 1, 0 };
    size_t i = 0;

    for (; i < content->data.length && position.line < start; i++) {
        file_pos_putc(&position, content->data.value[i]);
    }
    assert(position.line == start);
    size_t start_id = i;

    for (; i < content->data.length && position.line <= end; i++) {
        file_pos_putc(&position, content->data.value[i]);
    }
    if (position.line > end) {
        position.line--;
        i--;
    }
    assert(position.line == end);

    return file_pos_lines_handle(
        slice_new(&content->data.value[start_id], i - start_id),
        start, end
    );
}

static inline FileInLinesView file_in_lines_view(FilePosLinesHandle handle, Slice slice) {
    FileInLinesView view = {
        .handle = handle,
        .slice = slice,
    };
    return view;
}

FileInLinesView file_content_get_in_lines_view(const FileContent *content, Slice slice) {
    FileLoc loc = file_content_locate(content, slice);
    return file_in_lines_view(
        file_content_get_lines(content, loc.begin.line, loc.end.line),
        slice
    );
}

void file_content_free(FileContent *content) {
    if (content->allocated) {
        free((void*)content->data.value);
    }
    free(content);
}

#define PREFIX "  " ANSI_GRAY ">" ANSI_RESET " "

void file_pos_print(va_list list) {
    FilePos pos = va_arg(list, FilePos);
    printf("%ld:%ld", pos.line, pos.character);
}

void file_in_lines_view_print(va_list list) {
    FileInLinesView view = va_arg(list, FileInLinesView);
    assert(view.handle.content.value <= view.slice.value);
    bool enabled = false;
    size_t highlight_len = view.slice.length;
    printf(PREFIX);
    for (size_t i = 0; i < view.handle.content.length; i++) {
        if (!enabled && &view.handle.content.value[i] == view.slice.value) {
            printf(ANSI_RED);
            enabled = true;
        }
        char c = view.handle.content.value[i];
        if (c == '\n') {
            if (enabled) {
                printf(ANSI_RESET);
            }
        }
        putchar(c);
        if (c == '\n') {
            printf(PREFIX);
            if (enabled && highlight_len > 0) {
                printf(ANSI_RED);
            }
        }
        if (enabled && highlight_len > 0 && (--highlight_len == 0)) {
            printf(ANSI_RESET);
        }
    }
    assert(!enabled || highlight_len == 0);
}
