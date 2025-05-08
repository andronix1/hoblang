#include "type.h"
#include "ast/type.h"
#include "core/mempool.h"
#include "core/null.h"
#include "core/vec.h"
#include "lexer/token.h"
#include "parser/nodes/path.h"
#include "parser/parser.h"

AstType *parse_type(Parser *parser) {
    Token token = parser_peek(parser);
    switch (token.kind) {
        case TOKEN_STRUCT: {
            parser_take(parser);
            PARSER_EXPECT_NEXT(parser, TOKEN_OPENING_FIGURE_BRACE);
            AstStructField *fields = vec_new_in(parser->mempool, AstStructField);
            while (!parser_next_should_be(parser, TOKEN_CLOSING_FIGURE_BRACE)) {
                Slice name = PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice;
                PARSER_EXPECT_NEXT(parser, TOKEN_COLON);
                vec_push(fields, ast_struct_field_new(name, NOT_NULL(parse_type(parser))););
                if (!parser_check_list_sep(parser, TOKEN_CLOSING_FIGURE_BRACE)) return NULL;
            }
            return ast_type_new_struct(parser->mempool, fields);
        }
        case TOKEN_IDENT:
            return ast_type_new_path(parser->mempool, NOT_NULL(parse_path(parser)));
        default:
            parser_err(parser, token.slice, "expected type");
            return NULL;
    }
}
