#include "extern.h"
#include "ast/node.h"
#include "core/null.h"
#include "lexer/token.h"
#include "parser/nodes/fun.h"
#include "parser/nodes/value.h"
#include "parser/parser.h"

AstNode *parse_extern_node(Parser *parser, bool is_public) {
    OptSlice alias = opt_slice_new_null();
    if (parser_next_should_be(parser, TOKEN_OPENING_CIRCLE_BRACE)) {
        alias = opt_slice_new_value(PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice);
        PARSER_EXPECT_NEXT(parser, TOKEN_CLOSING_CIRCLE_BRACE);
    }
    Token token = parser_take(parser);
    switch (token.kind) {
        case TOKEN_FUN: {
            AstFunInfo *info = NOT_NULL(parse_fun_info(parser, is_public));
            PARSER_EXPECT_NEXT(parser, TOKEN_SEMICOLON);
            return ast_node_new_external_fun(parser->mempool, info, alias);
        }
        case TOKEN_VAR: {
            parser_skip_next(parser);
            AstValueInfo *info = NOT_NULL(parse_value_info(parser, is_public));
            PARSER_EXPECT_NEXT(parser, TOKEN_SEMICOLON);
            return ast_node_new_external_value(parser->mempool, info, alias);
        }
        default:
            parser_err(parser, token.slice, "expected extern info");
            return NULL;
    }
}
