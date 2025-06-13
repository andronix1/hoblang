#include "extern.h"
#include "ast/node.h"
#include "core/null.h"
#include "lexer/token.h"
#include "parser/nodes/fun.h"
#include "parser/nodes/value.h"
#include "parser/parser.h"

AstNode *parse_extern_node(Parser *parser, bool is_public) {
    Slice alias;
    bool has_alias = has_alias = parser_next_should_be(parser, TOKEN_OPENING_CIRCLE_BRACE);
    if (has_alias) {
        alias = PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice;
        PARSER_EXPECT_NEXT(parser, TOKEN_CLOSING_CIRCLE_BRACE);
    }
    Token token = parser_take(parser);
    switch (token.kind) {
        case TOKEN_FUN: {
            AstFunInfo *info = NOT_NULL(parse_fun_info(parser, is_public));
            PARSER_EXPECT_NEXT(parser, TOKEN_SEMICOLON);
            return ast_node_new_external_fun(parser->mempool, info, has_alias, alias);
        }
        case TOKEN_VAR: {
            AstValueInfo *info = NOT_NULL(parse_value_info(parser, is_public));
            PARSER_EXPECT_NEXT(parser, TOKEN_SEMICOLON);
            return ast_node_new_external_value(parser->mempool, info, has_alias, alias);
        }
        default:
            parser_err(parser, token.slice, "expected extern info");
            return NULL;
    }
}
