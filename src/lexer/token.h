#pragma once

#include "core/slice.h"
#include <stdarg.h>
#include <stdint.h>

typedef enum {
    // Keywords
    TOKEN_TYPE,
    TOKEN_STRUCT,
    TOKEN_PUBLIC,
    TOKEN_FUN,
    TOKEN_VAR, TOKEN_FINAL, TOKEN_CONST,
    TOKEN_GLOBAL, TOKEN_EXTERN,
    TOKEN_RETURN,
    TOKEN_IF, TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_IMPORT,
    TOKEN_CONTINUE, TOKEN_BREAK,
    TOKEN_AS,
    TOKEN_DO,
    TOKEN_USE,
    TOKEN_TRUE, TOKEN_FALSE,
    // Extended
    TOKEN_IDENT,
    TOKEN_CHAR, TOKEN_INTEGER, TOKEN_STRING, TOKEN_FLOAT,
    // Binops
    TOKEN_MINUS, TOKEN_SUBTRACT,
    TOKEN_STAR, TOKEN_MULTIPLY,
    TOKEN_SLASH, TOKEN_DIVIDE,
    TOKEN_PLUS, TOKEN_APPEND,
    TOKEN_EQUALS, TOKEN_NOT_EQUALS,
    TOKEN_GREATER_EQ, TOKEN_LESS_EQ,
    TOKEN_MOD,
    TOKEN_ASSIGN,
    TOKEN_AND, TOKEN_OR,
    TOKEN_BITAND, TOKEN_BITOR,
    TOKEN_NOT,
    // Braces
    TOKEN_OPENING_FIGURE_BRACE, TOKEN_CLOSING_FIGURE_BRACE,
    TOKEN_OPENING_CIRCLE_BRACE, TOKEN_CLOSING_CIRCLE_BRACE,
    TOKEN_OPENING_SQUARE_BRACE, TOKEN_CLOSING_SQUARE_BRACE,
    TOKEN_OPENING_ANGLE_BRACE, TOKEN_CLOSING_ANGLE_BRACE,
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

    union {
        Slice string;
        uint64_t integer;
        long double float_value;
        char character;
    };
} Token;

void token_print(va_list list);

static inline Token token_char(char c) {
    Token token = { .kind = TOKEN_CHAR, .character = c };
    return token;
}

static inline Token token_float(long double value) {
    Token token = { .kind = TOKEN_FLOAT, .float_value = value };
    return token;
}

static inline Token token_integer(uint64_t integer) {
    Token token = { .kind = TOKEN_INTEGER, .integer = integer };
    return token;
}

static inline Token token_string(Slice string) {
    Token token = { .kind = TOKEN_STRING, .string = string };
    return token;
}

static inline Token token_simple(TokenKind kind) {
    Token token = { .kind = kind };
    return token;
}
