#include "global.h"
#include "ast/global.h"
#include "lexer/token.h"
#include "parser/parser.h"

AstGlobal *parse_global(Parser *parser) {
    if (parser_next_should_be(parser, TOKEN_OPENING_CIRCLE_BRACE)) {
        Slice alias = PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice;
        PARSER_EXPECT_NEXT(parser, TOKEN_CLOSING_CIRCLE_BRACE);
        return ast_global_new_with_alias(parser->mempool, alias);
    }
    return ast_global_new(parser->mempool);
}
