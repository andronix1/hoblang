#include "lexer.h"
#include "core/buffer.h"
#include "core/file_content.h"
#include "core/log.h"
#include "core/slice.h"
#include <stdio.h>

char lexer_next_char(Lexer *lexer) {
    if (lexer->pos >= lexer->content->data.length) {
        lexer->pos++;
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

void lexer_err(Lexer* lexer, size_t before, const char *fmt, ...) {
    va_list list;
    va_start(list, fmt);
    logs("$s:$L: error: ", lexer->content->path, file_content_locate_pos(lexer->content, lexer->start_pos));
    logvln(fmt, list);
    logln("$V", file_content_get_in_lines_view(
        lexer->content, subslice(lexer->content->data, lexer->start_pos, before)));
    va_end(list);
}
