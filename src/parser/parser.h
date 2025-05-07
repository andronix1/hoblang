#pragma once

#include "core/mempool.h"
#include "lexer/api.h"
#include "lexer/token.h"

typedef struct Parser {
    Mempool *mempool;
    Lexer *lexer;
    bool lexer_ownership;

    Token cache;
    bool skip_next;
} Parser;

Token parser_take(Parser *parser);
bool parser_next_is(Parser *parser, TokenKind kind);
bool parser_next_is_not(Parser *parser, TokenKind kind);
void parser_err(Parser *parser, Slice at, const char *fmt, ...);

#define PARSER_EXPECT_NEXT(parser, kind) ({ \
    if (!parser_next_is(parser, kind)) { return NULL }; \
    parser_take(); \
})
