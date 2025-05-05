#include "api.h"
#include "core/buffer.h"
#include "lexer/lexer.h"
#include "core/file_content.h"
#include "lexer/token.h"
#include <stdio.h>
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

static inline bool char_is_digit(char c) {
    return c >= '0' && c <= '9';
}

static inline bool char_is_ident_start(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '$';
}

static inline bool char_is_ident(char c) {
    return char_is_ident_start(c) || char_is_digit(c);
}

static Token _lexer_next(Lexer *lexer) {
    while (true) {
        char c = lexer_next_char(lexer);
        switch (c) {
            case '+':
                if (lexer_next_char_is(lexer, '=')) return token_simple(TOKEN_APPEND);
                return token_simple(TOKEN_PLUS);
            case '-':
                if (lexer_next_char_is(lexer, '=')) return token_simple(TOKEN_SUBTRACT);
                return token_simple(TOKEN_MINUS);
            case EOF:
                return token_simple(TOKEN_EOI);
            default:
                if (char_is_ident_start(c)) {
                    while (char_is_ident(lexer_next_char(lexer)));
                    return token_simple(TOKEN_IDENT);
                }
                printf("error: invalid token\n");
                break;
        }
    }
}

Token lexer_next(Lexer *lexer) {
    Token result = _lexer_next(lexer);
    result.slice = lexer_slice(lexer);
    lexer_mark_parsed(lexer);
    return result;
}

void lexer_free(Lexer *lexer) {
    if (lexer->internal_content) {
        file_content_free((FileContent*)lexer->content);
    }
    buffer_free(&lexer->buffer);
    free(lexer);
}
