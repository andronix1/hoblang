#include "file_content.h"
#include "core/slice.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

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

    FileContent *result = malloc(sizeof(FileContent));
    *(Path*)&result->path = path;
    *(Slice*)&result->content = slice_new(content, size);
    return result;
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
    assert(slice.value > content->content.value);
    size_t position = slice.value - content->content.value;
    assert(position < content->content.length);
    FileLoc result = {
        .begin = { .line = 1, .character = 0 }
    };
    for (size_t i = 0; i <= position; i++) {
        file_pos_putc(&result.begin, content->content.value[i]);
    }
    result.end = result.begin;
    for (size_t i = 1; i < slice.length; i++) {
        file_pos_putc(&result.end, slice.value[i]);
    }
    return result;
}

void file_content_free(FileContent *content) {
    free((void*)content->content.value);
    free(content);
}
