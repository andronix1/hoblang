#include "ast/global.h"
#include "ast/node.h"
#include "core/assert.h"
#include "lexer/token.h"
#include "parser/api.h"
#include "parser/nodes/expr.h"
#include "parser/nodes/type.h"
#include "parser/parser.h"

AstValueInfo *parse_value_info(Parser *parser, bool is_public) {
    Token kind_token = parser_take(parser);
    AstValueDeclKind kind;
    switch (kind_token.kind) {
        case TOKEN_VAR: kind = AST_VALUE_DECL_VAR; break;
        case TOKEN_FINAL: kind = AST_VALUE_DECL_FINAL; break;
        case TOKEN_CONST: kind = AST_VALUE_DECL_CONST; break;
        default: UNREACHABLE;
    }
    Slice name = PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice;
    AstType *explicit_type = NULL;
    if (parser_next_should_be(parser, TOKEN_COLON)) {
        explicit_type = parse_type(parser);
    }
    return ast_value_info_new(parser->mempool, is_public, kind, name, explicit_type);
}

AstNode *parse_value_decl_node(Parser *parser, AstGlobal *global, bool is_public) {
    AstValueInfo *info = parse_value_info(parser, is_public);
    AstExpr *initializer = NULL;
    if (parser_next_should_be(parser, TOKEN_ASSIGN)) {
        initializer = parse_expr(parser);
    }
    PARSER_EXPECT_NEXT(parser, TOKEN_SEMICOLON);
    return ast_node_new_value_decl(parser->mempool, global, info, initializer);
}
