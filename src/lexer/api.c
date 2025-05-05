#include "api.h"
#include "core/buffer.h"
#include "lexer/lexer.h"
#include "core/file_content.h"
#include <stdlib.h>

static inline Lexer *lexer_new(const FileContent *content, bool internal_content) {
    Lexer *lexer = malloc(sizeof(Lexer));
    lexer->content = content;
    lexer->internal_content = internal_content;
    lexer->buffer = buffer_new();
    lexer->start_pos = lexer->pos = 0;
    return lexer;
}

Lexer *lexer_from_file_content(const FileContent *content) {
    return lexer_new(content, false);
}

Lexer *lexer_from_file(const Path path) {
    FileContent *content = file_content_read(path);
    if (!content) {
        return NULL;
    }
    return lexer_new(content, true);
}

void lexer_free(Lexer *lexer) {
    if (lexer->internal_content) {
        file_content_free((FileContent*)lexer->content);
    }
    buffer_free(&lexer->buffer);
    free(lexer);
}
