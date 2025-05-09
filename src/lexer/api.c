#include "api.h"
#include "core/buffer.h"
#include "lexer/lexer.h"
#include "core/file_content.h"
#include <stdlib.h>

Lexer *lexer_new(const FileContent *content, bool content_ownership) {
    Lexer *lexer = malloc(sizeof(Lexer));
    lexer->content = content;
    lexer->content_ownership = content_ownership;
    lexer->buffer = buffer_new();
    lexer->start_pos = lexer->pos = 0;
    return lexer;
}

void lexer_free(Lexer *lexer) {
    if (lexer->content_ownership) {
        file_content_free((FileContent*)lexer->content);
    }
    buffer_free(&lexer->buffer);
    free(lexer);
}
