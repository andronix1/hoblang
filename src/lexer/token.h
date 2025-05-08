#pragma once

#include "core/slice.h"
#include <stdarg.h>

typedef enum {
    // Keywords
    TOKEN_TYPE,
    TOKEN_STRUCT,
    TOKEN_LOCAL,
    TOKEN_FUN,
    TOKEN_VAR, TOKEN_FINAL, TOKEN_CONST,
    TOKEN_GLOBAL, TOKEN_EXTERN,
    // Extended
    TOKEN_IDENT,
    // Binops
    TOKEN_MINUS, TOKEN_SUBTRACT,
    TOKEN_PLUS, TOKEN_APPEND,
    TOKEN_ASSIGN,
    // Braces
    TOKEN_OPENING_FIGURE_BRACE, TOKEN_CLOSING_FIGURE_BRACE,
    TOKEN_OPENING_CIRCLE_BRACE, TOKEN_CLOSING_CIRCLE_BRACE,
    // Punctuation
    TOKEN_COLON, TOKEN_SEMICOLON,
    TOKEN_COMMA, TOKEN_DOT,
    TOKEN_FUN_RETURNS,
    // Utility
    TOKEN_FAILED,
    TOKEN_EOI,
} TokenKind;

typedef struct {
    TokenKind kind;
    Slice slice;
} Token;

void token_print(va_list list);

static inline Token token_simple(TokenKind kind) {
    Token token = { .kind = kind };
    return token;
}
