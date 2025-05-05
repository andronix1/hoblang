#include "file_content.h"
#include "core/slice.h"
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

void file_content_free(FileContent *content) {
    free((void*)content->content.value);
    free(content);
}
