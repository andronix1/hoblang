#include "api.h"
#include "core/buffer.h"
#include "core/mempool.h"
#include "lexer/lexer.h"
#include "core/file_content.h"
#include <stdlib.h>

Lexer *lexer_new(const FileContent *content) {
    Lexer *lexer = malloc(sizeof(Lexer));
    lexer->content = content;
    lexer->buffer = buffer_new();
    lexer->mempool = mempool_new(128);
    lexer->start_pos = lexer->pos = 0;
    return lexer;
}

void lexer_free(Lexer *lexer) {
    file_content_free((FileContent*)lexer->content);
    mempool_free(lexer->mempool);
    buffer_free(&lexer->buffer);
    free(lexer);
}
