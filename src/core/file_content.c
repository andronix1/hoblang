#include "file_content.h"
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

FileLoc file_content_locate(FileContent *content, Slice slice) {
    assert(slice.value > content->data.value);
    size_t position = slice.value - content->data.value;
    assert(position < content->data.length);
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

void file_content_free(FileContent *content) {
    if (content->allocated) {
        free((void*)content->data.value);
    }
    free(content);
}
