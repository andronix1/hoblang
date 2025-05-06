#pragma once

#include "core/slice.h"

#define TOKEN_FAILED -2

typedef enum {
    TOKEN_IDENT,
    TOKEN_MINUS, TOKEN_SUBTRACT,
    TOKEN_PLUS, TOKEN_APPEND,
    TOKEN_EOI,
} TokenKind;

typedef struct {
    TokenKind kind;
    Slice slice;
} Token;

static inline Token token_simple(TokenKind kind) {
    Token token = { .kind = kind };
    return token;
}
