#include "parser.h"
#include "core/log.h"
#include "lexer/api.h"
#include "lexer/lexer.h"
#include "lexer/token.h"

Token parser_take(Parser *parser) {
    if (parser->skip_next) {
        parser->skip_next = false;
        return parser->cache;
    }
    parser->cache = lexer_next(parser->lexer);
    assert(parser->cache.kind != TOKEN_FAILED);
    return parser->cache;
}

inline Token parser_peek(Parser *parser) {
    Token token = parser_take(parser);
    parser_skip_next(parser);
    return token;
}

inline void parser_skip_next(Parser *parser) {
    assert(!parser->skip_next);
    parser->skip_next = true;
}

inline bool parser_next_is(Parser *parser, TokenKind kind) {
    if (parser_peek(parser).kind != kind) {
        return false;
    }
    return true;
}

bool parser_next_is_not(Parser *parser, TokenKind kind) {
    if (parser_peek(parser).kind != kind) {
        return true;
    }
    return false;
}

inline bool parser_next_should_be(Parser *parser, TokenKind kind) {
    if (parser_next_is(parser, kind)) {
        parser_take(parser);
        return true;
    }
    return false;
}

inline bool parser_next_should_not_be(Parser *parser, TokenKind kind) {
    if (parser_next_is_not(parser, kind)) {
        parser_take(parser);
        return true;
    }
    return false;
}

bool parser_check_list_sep(Parser *parser, TokenKind end) {
    if (parser_next_should_be(parser, TOKEN_COMMA) || parser_next_is(parser, end)) {
        return true;
    }
    parser_err(parser, parser->cache.slice, "expected separator");
    return false;
}

void parser_err(Parser *parser, Slice at, const char *fmt, ...) {
    va_list list;
    va_start(list, fmt);
    logs("$s:$L: error: ",
        parser->lexer->content->path,
        file_content_locate(parser->lexer->content, at).begin);
    logvln(fmt, list);
    logln("$V\n", file_content_get_in_lines_view(parser->lexer->content, at));
    va_end(list);
}
