#include "parser.h"
#include "core/log.h"
#include "lexer/api.h"
#include "lexer/lexer.h"
#include <stdio.h>

Token parser_take(Parser *parser) {
    if (parser->skip_next) {
        parser->skip_next = false;
        return parser->cache;
    }
    return parser->cache = lexer_next(parser->lexer);
}

bool parser_next_is(Parser *parser, TokenKind kind) {
    if (parser_take(parser).kind != kind) {
        parser->skip_next = true;
        return false;
    }
    return true;
}

bool parser_next_is_not(Parser *parser, TokenKind kind) {
    if (parser_take(parser).kind == kind) {
        parser->skip_next = true;
        return true;
    }
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
