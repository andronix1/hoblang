#include "type_decl.h"
#include "ast/api/generic.h"
#include "ast/node.h"
#include "core/null.h"
#include "lexer/token.h"
#include "parser/nodes/generic.h"
#include "parser/nodes/type.h"
#include "parser/parser.h"

AstNode *parse_type_decl_node(Parser *parser, bool is_local) {
    Slice name = PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice;
    AstGeneric *generic = NULL;
    if (parser_next_is(parser, TOKEN_OPENING_ANGLE_BRACE)) {
        generic = NOT_NULL(parse_generic(parser));
    }
    PARSER_EXPECT_NEXT(parser, TOKEN_ASSIGN);
    AstType *type = NOT_NULL(parse_type(parser));
    PARSER_EXPECT_NEXT(parser, TOKEN_SEMICOLON);
    return ast_node_new_type_decl(parser->mempool, is_local, name, generic, type);
}
