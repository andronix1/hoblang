#include "lexer.h"
#include "core/buffer.h"
#include "core/slice.h"
#include <stdio.h>

char lexer_next_char(Lexer *lexer) {
    if (lexer->pos >= lexer->content->data.length) {
        return EOF;
    }
    char c = lexer->content->data.value[lexer->pos++];
    buffer_putc(&lexer->buffer, c);
    return c;
}

bool lexer_next_char_is(Lexer *lexer, char c) {
    if (lexer->pos >= lexer->content->data.length) {
        return c == EOF;
    }
    if (lexer->content->data.value[lexer->pos] == c) {
        lexer->pos++;
        buffer_putc(&lexer->buffer, c);
        return true;
    }
    return false;
}

Slice lexer_slice(const Lexer *lexer) {
    return slice_new(&lexer->content->data.value[lexer->start_pos], lexer->pos - lexer->start_pos);
}

void lexer_mark_parsed(Lexer *lexer) {
    buffer_erase(&lexer->buffer);
    lexer->start_pos = lexer->pos;
}
