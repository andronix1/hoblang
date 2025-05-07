#include "api.h"
#include "core/slice.h"
#include "lexer/lexer.h"
#include "lexer/token.h"
#include <stdio.h>

static inline bool char_is_digit(char c) {
    return c >= '0' && c <= '9';
}

static inline bool char_is_ident_start(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '$';
}

static inline bool char_is_ident(char c) {
    return char_is_ident_start(c) || char_is_digit(c);
}

static inline bool char_is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n';
}

static Token lexer_try_next(Lexer *lexer) {
    char c;
    lexer_mark_parsed(lexer);
    while(char_is_whitespace(c = lexer_next_char(lexer))) {
        lexer_mark_parsed(lexer);
    }
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
                lexer->pos--;
                Slice slice = lexer_slice(lexer);
                if (slice_eq(slice, slice_from_cstr("type"))) return token_simple(TOKEN_TYPE);
                else if (slice_eq(slice, slice_from_cstr("struct"))) return token_simple(TOKEN_STRUCT);
                else return token_simple(TOKEN_IDENT);
            }
            return token_simple(TOKEN_FAILED);
    }
}


Token lexer_next(Lexer *lexer) {
    Token result = lexer_try_next(lexer);
    if (result.kind == TOKEN_FAILED) {
        size_t sp = lexer->start_pos;
        size_t end = lexer->pos;
        while ((result = lexer_try_next(lexer)).kind == TOKEN_FAILED) {
            end = lexer->pos;
        }
        size_t spn = lexer->start_pos;
        lexer->start_pos = sp;
        lexer_err(lexer, end, "failed to recognize token");
        lexer->start_pos = spn;
    }
    result.slice = lexer_slice(lexer);
    lexer_mark_parsed(lexer);
    return result;
}
