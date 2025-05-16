#include "generic.h"
#include "ast/generic.h"
#include "core/mempool.h"
#include "lexer/token.h"
#include "parser/parser.h"

AstGeneric *parse_generic(Parser *parser) {
    PARSER_EXPECT_NEXT(parser, TOKEN_OPENING_ANGLE_BRACE);
    AstGenericParam *params = vec_new_in(parser->mempool, AstGenericParam);
    while (!parser_next_should_be(parser, TOKEN_CLOSING_ANGLE_BRACE)) {
        vec_push(params, ast_generic_param_new(PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice));
        if (!parser_check_list_sep(parser, TOKEN_CLOSING_ANGLE_BRACE)) return NULL;
    }
    return ast_generics_new(parser->mempool, params);
}
