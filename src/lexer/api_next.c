#include "api.h"
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

static Token _lexer_next(Lexer *lexer) {
    while (true) {
        char c;
        while(char_is_whitespace(c = lexer_next_char(lexer)));
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
