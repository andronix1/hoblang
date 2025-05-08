#include "body.h"
#include "ast/body.h"
#include "core/mempool.h"
#include "core/null.h"
#include "lexer/token.h"
#include "parser/api.h"
#include "parser/parser.h"

AstBody *parse_body(Parser *parser) {
    AstNode **nodes = vec_new_in(parser->mempool, AstNode*);
    PARSER_EXPECT_NEXT(parser, TOKEN_OPENING_FIGURE_BRACE);
    while (!parser_next_should_be(parser, TOKEN_CLOSING_FIGURE_BRACE)) {
        vec_push(nodes, NOT_NULL(parser_next(parser)));
    }
    return ast_body_new(parser->mempool, nodes);
}
