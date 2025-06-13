#include "fun.h"
#include "ast/api/body.h"
#include "ast/api/type.h"
#include "ast/node.h"
#include "core/mempool.h"
#include "core/null.h"
#include "core/slice.h"
#include "core/vec.h"
#include "lexer/token.h"
#include "parser/nodes/body.h"
#include "parser/nodes/type.h"
#include "parser/parser.h"
#include <stdio.h>

AstFunInfo *parse_fun_info(Parser *parser, bool is_public) {
    Slice name = PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice;
    Slice ext_of;
    bool is_ext;
    if ((is_ext = parser_next_should_be(parser, TOKEN_DOT))) {
        ext_of = name;
        name = PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice;
    }
    PARSER_EXPECT_NEXT(parser, TOKEN_OPENING_CIRCLE_BRACE);
    bool by_ref = false;
    Slice self_name = slice_new(NULL, 0);
    bool was_arg = false;
    if (is_ext) {
        by_ref = parser_next_should_be(parser, TOKEN_AND);
        self_name = PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice;
        was_arg = true;
    }
    AstFunArg *args = vec_new_in(parser->mempool, AstFunArg);
    while (!parser_next_should_be(parser, TOKEN_CLOSING_CIRCLE_BRACE)) {
        if (was_arg) {
            PARSER_EXPECT_NEXT(parser, TOKEN_COMMA);
            was_arg = false;
        }
        Slice arg_name = PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice;
        PARSER_EXPECT_NEXT(parser, TOKEN_COLON);
        vec_push(args, ast_fun_arg_new(arg_name, NOT_NULL(parse_type(parser))));
        if (!parser_check_list_sep(parser, TOKEN_CLOSING_CIRCLE_BRACE)) return NULL;
    }
    AstType *returns = parser_next_should_be(parser, TOKEN_FUN_RETURNS) ? NOT_NULL(parse_type(parser)) : NULL;
    return is_ext ?
        ast_ext_fun_info_new(parser->mempool, is_public, name, args, returns, ext_of, by_ref, self_name):
        ast_fun_info_new(parser->mempool, is_public, name, args, returns);
}

AstNode *parse_fun_decl_node(Parser *parser, AstGlobal *global, bool is_public) {
    AstFunInfo *info = NOT_NULL(parse_fun_info(parser, is_public));
    AstBody *body = NOT_NULL(parse_body(parser));
    return ast_node_new_fun_decl(parser->mempool, global, info, body);
}
