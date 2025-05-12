#include "api.h"
#include "ast/api/expr.h"
#include "ast/global.h"
#include "ast/node.h"
#include "ast/stmt.h"
#include "core/mempool.h"
#include "core/null.h"
#include "core/slice.h"
#include "core/vec.h"
#include "lexer/token.h"
#include "parser/nodes/expr.h"
#include "parser/nodes/extern.h"
#include "parser/nodes/fun.h"
#include "parser/nodes/global.h"
#include "parser/nodes/type_decl.h"
#include "parser/nodes/value.h"
#include "parser/parser.h"

#define NOT_FOUND ((void*)-1)

static AstNode *parser_next_maybe_local_and_global(Parser *parser, AstGlobal *global, bool is_local) {
    Token token = parser_take(parser);
    switch (token.kind) {
        case TOKEN_FUN: return parse_fun_decl_node(parser, global, is_local);
        default: return NOT_FOUND;
    }
}

static AstNode *parser_next_maybe_local(Parser *parser, Token token, bool is_local) {
    switch (token.kind) {
        case TOKEN_TYPE:
            return parse_type_decl_node(parser, is_local);
        case TOKEN_VAR: case TOKEN_FINAL: case TOKEN_CONST:
            parser_skip_next(parser);
            return parse_value_decl_node(parser, is_local);
        case TOKEN_GLOBAL:
            return parser_next_maybe_local_and_global(parser, NOT_NULL(parse_global(parser)), is_local);
        case TOKEN_EXTERN:
            return parse_extern_node(parser, is_local);
        default:
            parser_skip_next(parser);
            return parser_next_maybe_local_and_global(parser, NULL, is_local);
    }
}

static AstNode *parser_next_full(Parser *parser, Token token) {
    switch (token.kind) {
        case TOKEN_IDENT:
        case TOKEN_INTEGER:
        case TOKEN_OPENING_CIRCLE_BRACE:
            parser_skip_next(parser);
            AstExpr *expr = parse_expr(parser);
            PARSER_EXPECT_NEXT(parser, TOKEN_SEMICOLON);
            return ast_node_new_stmt(parser->mempool, ast_stmt_new_expr(parser->mempool, expr));
        case TOKEN_RETURN: {
            AstExpr *value = parser_next_is(parser, TOKEN_SEMICOLON) ?
                NULL : NOT_NULL(parse_expr(parser));
            PARSER_EXPECT_NEXT(parser, TOKEN_SEMICOLON);
            return ast_node_new_stmt(parser->mempool, ast_stmt_new_return(parser->mempool, token.slice, value));
        }
        default: return parser_next_maybe_local(parser, token, false);
    }
}

AstNode *parser_next(Parser *parser) {
    while (true) {
        bool is_local = parser_next_should_be(parser, TOKEN_LOCAL);
        Token token = parser_take(parser);
        if (token.kind == TOKEN_EOI) {
            return NULL;
        }
        AstNode *try = is_local ?
            parser_next_maybe_local(parser, token, true) :
            parser_next_full(parser, token);
        if (try == NOT_FOUND) {
            parser_err(parser, token.slice, "unexpected token $T", token);
        } else if (try != NULL) {
            try->slice = slice_union(token.slice, parser->cache.slice);
            return try;
        }
    }
}

AstNode **parser_parse(Parser *parser) {
    AstNode **nodes = vec_new_in(parser->mempool, AstNode*);
    AstNode *node;
    while ((node = parser_next(parser))) {
        vec_push(nodes, node);
    }
    return nodes;
}
