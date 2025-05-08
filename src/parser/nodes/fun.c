#include "fun.h"
#include "ast/api/type.h"
#include "ast/node.h"
#include "core/mempool.h"
#include "core/null.h"
#include "core/vec.h"
#include "lexer/token.h"
#include "parser/nodes/type.h"
#include "parser/parser.h"

AstNode *parse_fun_decl_node(Parser *parser, bool is_local) {
    Slice name = PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice;
    PARSER_EXPECT_NEXT(parser, TOKEN_OPENING_CIRCLE_BRACE);
    AstFunArg *args = vec_new_in(parser->mempool, AstFunArg);
    while (!parser_next_should_be(parser, TOKEN_CLOSING_CIRCLE_BRACE)) {
        Slice arg_name = PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice;
        PARSER_EXPECT_NEXT(parser, TOKEN_COLON);
        vec_push(args, ast_node_fun_arg(arg_name, NOT_NULL(parse_type(parser))));
        if (!parser_check_list_sep(parser, TOKEN_CLOSING_CIRCLE_BRACE)) return NULL;
    }
    AstType *returns = parser_next_should_be(parser, TOKEN_FUN_RETURNS) ?
        NOT_NULL(parse_type(parser)) : NULL;
    PARSER_EXPECT_NEXT(parser, TOKEN_OPENING_FIGURE_BRACE);
    PARSER_EXPECT_NEXT(parser, TOKEN_CLOSING_FIGURE_BRACE);
    return ast_node_new_fun_decl(parser->mempool, is_local, name, args, returns);
}
