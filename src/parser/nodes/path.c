#include "path.h"
#include "ast/generic.h"
#include "ast/path.h"
#include "ast/type.h"
#include "core/mempool.h"
#include "core/null.h"
#include "core/slice.h"
#include "lexer/token.h"
#include "parser/nodes/type.h"
#include "parser/parser.h"

AstPath *parse_path(Parser *parser) {
    AstPathSegment *segments = vec_new_in(parser->mempool, AstPathSegment);
    do {
        Token token = parser_take(parser);
        switch (token.kind) {
            case TOKEN_IDENT: vec_push(segments, ast_path_segment_new_ident(token.slice)); break;
            case TOKEN_STAR: vec_push(segments, ast_path_segment_new_deref(token.slice)); break;
            case TOKEN_OPENING_ANGLE_BRACE: {
                AstType **params = vec_new_in(parser->mempool, AstType*);
                Slice slice = token.slice;
                while (parser_next_is_not(parser, TOKEN_CLOSING_ANGLE_BRACE)) {
                    vec_push(params, NOT_NULL(parse_type(parser)));
                    if (!parser_check_list_sep(parser, TOKEN_CLOSING_ANGLE_BRACE)) return NULL;
                }
                slice = slice_union(slice, parser_take(parser).slice);
                vec_push(segments, ast_path_segment_new_generic_build(slice,
                    ast_generic_builder_new(parser->mempool, params))); break;
            }
            default:
                parser_err(parser, token.slice, "expected path segment");
                return NULL;
        }
    } while (parser_next_should_be(parser, TOKEN_DOT));
    return ast_path_new(parser->mempool, segments);
}
