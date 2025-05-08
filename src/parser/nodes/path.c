#include "path.h"
#include "ast/path.h"
#include "core/mempool.h"
#include "lexer/token.h"
#include "parser/parser.h"

AstPath *parse_path(Parser *parser) {
    AstPathSegment *segments = vec_new_in(parser->mempool, AstPathSegment);
    do {
        Token token = parser_take(parser);
        switch (token.kind) {
            case TOKEN_IDENT: vec_push(segments, ast_path_segment_new_ident(token.slice)); break;
            default:
                parser_err(parser, token.slice, "expected path segment");
                return NULL;
        }
    } while (parser_next_should_be(parser, TOKEN_DOT));
    return ast_path_new(parser->mempool, segments);
}
