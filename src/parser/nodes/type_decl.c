#include "type_decl.h"
#include "ast/node.h"
#include "core/null.h"
#include "lexer/token.h"
#include "parser/nodes/type.h"
#include "parser/parser.h"

AstNode *parse_type_decl_node(Parser *parser) {
    Slice name = PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice;
    PARSER_EXPECT_NEXT(parser, TOKEN_ASSIGN);
    return ast_node_new_type_decl(parser->mempool, name, NOT_NULL(parse_type(parser)));
}
