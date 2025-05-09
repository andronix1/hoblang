#pragma once

#include "core/mempool.h"
#include "lexer/api.h"
#include "lexer/token.h"

typedef struct Parser {
    Mempool *mempool;
    Lexer *lexer;

    Token cache;
    bool skip_next;
} Parser;

Token parser_take(Parser *parser);
Token parser_peek(Parser *parser);
void parser_skip_next(Parser *parser);
bool parser_next_is(Parser *parser, TokenKind kind);
bool parser_next_is_not(Parser *parser, TokenKind kind);
bool parser_next_should_be(Parser *parser, TokenKind kind);
bool parser_next_should_not_be(Parser *parser, TokenKind kind);
bool parser_check_list_sep(Parser *parser, TokenKind end);
void parser_err(Parser *parser, Slice at, const char *fmt, ...);

#define PARSER_EXPECT_NEXT(PARSER, KIND) ({ \
    if (parser_next_is_not(PARSER, KIND)) { \
        parser_err(PARSER, parser_peek(PARSER).slice, "expected $T", token_simple(KIND)); \
        return NULL; \
    } \
    parser_take(PARSER); \
})
