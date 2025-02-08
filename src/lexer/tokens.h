#pragma once

#include <stdbool.h>
#include <printf.h>
#include <stdint.h>
#include "core/vec.h"
#include "core/location.h"
#include "core/slice.h"

typedef enum { 
	TOKEN_FUN,
	TOKEN_DEFER,
	TOKEN_TYPE,
	TOKEN_IMPORT,
	TOKEN_USE,
	TOKEN_WHILE,
    TOKEN_VAR,
    TOKEN_RETURN,
    TOKEN_EXTERN,
    TOKEN_AS,
    TOKEN_ASM,
    TOKEN_DOLLAR,
    TOKEN_VOLATILE,
	TOKEN_CONST,
    
	TOKEN_STRUCT,
    TOKEN_FUNC_RETURNS,
    
	TOKEN_IF,
    TOKEN_ELSE,

    TOKEN_COLON,
    TOKEN_DOUBLE_COLON,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_ASSIGN,

	TOKEN_OPENING_CIRCLE_BRACE,
	TOKEN_CLOSING_CIRCLE_BRACE,
	TOKEN_OPENING_FIGURE_BRACE,
	TOKEN_CLOSING_FIGURE_BRACE,
	TOKEN_OPENING_SQUARE_BRACE,
	TOKEN_CLOSING_SQUARE_BRACE,
    
	TOKEN_ADD,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_NOT,
    TOKEN_DOT,

    TOKEN_BITOR,
    TOKEN_BITAND,
    TOKEN_BITNOT,
    TOKEN_XOR,
    TOKEN_SHR,
    TOKEN_SHL,

    TOKEN_AND,
    TOKEN_OR,

    TOKEN_EQUALS,
    TOKEN_NOT_EQUALS,
    TOKEN_LESS,
    TOKEN_LESS_OR_EQUALS,
    TOKEN_GREATER,
    TOKEN_GREATER_OR_EQUALS,

    TOKEN_TRUE,
    TOKEN_FALSE,

    TOKEN_CHAR,
    TOKEN_INTEGER,
    TOKEN_IDENT,
    TOKEN_STR,
	
	TOKEN_EOI 
} TokenType;

typedef struct {
    TokenType type;
    FileLocation location;
    union {
        Slice ident;
        uint64_t integer;
		char character;
        char *str;
    };
} Token;

typedef struct {
	const Token *tokens;
	size_t len;
} TokensSlice;

void print_token(FILE *stream, va_list list);
