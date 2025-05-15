#include "type.h"
#include "ast/type.h"
#include "core/keymap.h"
#include "core/mempool.h"
#include "core/null.h"
#include "lexer/token.h"
#include "parser/nodes/path.h"
#include "parser/parser.h"

AstType *parse_type(Parser *parser) {
    Token token = parser_take(parser);
    switch (token.kind) {
        case TOKEN_STRUCT: {
            PARSER_EXPECT_NEXT(parser, TOKEN_OPENING_FIGURE_BRACE);
            AstStructField *fields = keymap_new_in(parser->mempool, AstStructField);
            while (!parser_next_should_be(parser, TOKEN_CLOSING_FIGURE_BRACE)) {
                bool is_local = parser_next_should_be(parser, TOKEN_LOCAL);
                Slice name = PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice;
                PARSER_EXPECT_NEXT(parser, TOKEN_COLON);
                if (keymap_insert(fields, name, ast_struct_field_new(is_local, NOT_NULL(parse_type(parser))))) {
                    parser_err(parser, name, "duplicate field");
                }
                if (!parser_check_list_sep(parser, TOKEN_CLOSING_FIGURE_BRACE)) return NULL;
            }
            return ast_type_new_struct(parser->mempool, fields);
        }
        case TOKEN_IDENT:
            parser_skip_next(parser);
            return ast_type_new_path(parser->mempool, NOT_NULL(parse_path(parser)));
        case TOKEN_STAR:
            return ast_type_new_pointer(parser->mempool, NOT_NULL(parse_type(parser)));
        case TOKEN_OPENING_SQUARE_BRACE:
            PARSER_EXPECT_NEXT(parser, TOKEN_CLOSING_SQUARE_BRACE);
            return ast_type_new_slice(parser->mempool, NOT_NULL(parse_type(parser)));
        default:
            parser_err(parser, token.slice, "expected type");
            return NULL;
    }
}
