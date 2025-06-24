#include "api.h"
#include "ast/api/expr.h"
#include "ast/body.h"
#include "ast/expr.h"
#include "ast/global.h"
#include "ast/node.h"
#include "ast/stmt.h"
#include "core/assert.h"
#include "core/mempool.h"
#include "core/null.h"
#include "core/opt_slice.h"
#include "core/slice.h"
#include "core/vec.h"
#include "lexer/token.h"
#include "parser/nodes/body.h"
#include "parser/nodes/expr.h"
#include "parser/nodes/extern.h"
#include "parser/nodes/fun.h"
#include "parser/nodes/global.h"
#include "parser/nodes/stmts/if.h"
#include "parser/nodes/type_decl.h"
#include "parser/nodes/value.h"
#include "parser/parser.h"

#define NOT_FOUND ((void*)-1)

static AstNode *parser_next_maybe_public_and_global(Parser *parser, AstGlobal *global, bool is_public) {
    Token token = parser_take(parser);
    switch (token.kind) {
        case TOKEN_FUN: return parse_fun_decl_node(parser, global, is_public);
        case TOKEN_VAR: case TOKEN_FINAL: case TOKEN_CONST:
            parser_skip_next(parser);
            return parse_value_decl_node(parser, global, is_public);
        default: return NOT_FOUND;
    }
}

static AstModulePath *parse_module_path(Parser *parser) {
    Slice *module_path = vec_new_in(parser->mempool, Slice);
    Slice ident = PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice;
    while (parser_next_should_be(parser, TOKEN_DOT)) {
        vec_push(module_path, ident);
        Token next = parser_take(parser);
        switch (next.kind) {
            case TOKEN_IDENT:
                ident = next.slice;
                break;
            case TOKEN_OPENING_FIGURE_BRACE: {
                AstModulePath **paths = vec_new_in(parser->mempool, AstModulePath*);
                while (!parser_next_should_be(parser, TOKEN_CLOSING_FIGURE_BRACE)) {
                    vec_push(paths, NOT_NULL(parse_module_path(parser)));
                    if (!parser_check_list_sep(parser, TOKEN_CLOSING_FIGURE_BRACE)) return NULL;
                }
                return ast_module_path_combined(parser->mempool, module_path, paths);
            }
            default:
                parser_err(parser, next.slice, "expected module path segment");
                return NULL;
        }
    }
    if (parser_next_should_be(parser, TOKEN_AS)) {
        Slice alias = PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice;
        return ast_module_path_single(parser->mempool, module_path, ident, opt_slice_new_value(alias));
    }
    return ast_module_path_single(parser->mempool, module_path, ident, opt_slice_new_null());
}

static AstNode *parser_next_maybe_public(Parser *parser, Token token, bool is_public) {
    switch (token.kind) {
        case TOKEN_TYPE:
            return parse_type_decl_node(parser, is_public);
        case TOKEN_GLOBAL:
            return parser_next_maybe_public_and_global(parser, NOT_NULL(parse_global(parser)), is_public);
        case TOKEN_EXTERN:
            return parse_extern_node(parser, is_public);
        case TOKEN_USE: {
            AstModulePath *path = NOT_NULL(parse_module_path(parser));
            PARSER_EXPECT_NEXT(parser, TOKEN_SEMICOLON);
            return ast_node_new_use(parser->mempool, is_public, path);
        }
        case TOKEN_IMPORT: {
            Token what = parser_take(parser);
            switch (what.kind) {
                case TOKEN_STRING: {
                    if (parser_next_should_be(parser, TOKEN_AS)) {
                        Slice alias = PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice;
                        PARSER_EXPECT_NEXT(parser, TOKEN_SEMICOLON);
                        return ast_node_new_import_module_alias(parser->mempool, is_public, what.slice, what.string,
                            alias);
                    }
                    PARSER_EXPECT_NEXT(parser, TOKEN_SEMICOLON);
                    return ast_node_new_import_module(parser->mempool, is_public, what.slice, what.string);
                }
                case TOKEN_IDENT: {
                    if (parser_next_should_be(parser, TOKEN_AS)) {
                        Slice alias = PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice;
                        PARSER_EXPECT_NEXT(parser, TOKEN_SEMICOLON);
                        return ast_node_new_import_library_alias(parser->mempool, is_public, what.slice, alias);
                    }
                    PARSER_EXPECT_NEXT(parser, TOKEN_SEMICOLON);
                    return ast_node_new_import_library(parser->mempool, is_public, what.slice);
                }
                default:
                    parser_err(parser, what.slice, "expected module path or library name");
            }
            UNREACHABLE;
        }
        default:
            parser_skip_next(parser);
            return parser_next_maybe_public_and_global(parser, NULL, is_public);
    }
}

static inline bool parse_loop_control(Parser *parser, AstLoopControl *output) {
    if (parser_next_is(parser, TOKEN_IDENT)) {
        Slice label = parser_take(parser).slice;
        PARSER_EXPECT_NEXT(parser, TOKEN_SEMICOLON);
        *output = ast_loop_control_new_labeled(label);
        return true;
    }
    PARSER_EXPECT_NEXT(parser, TOKEN_SEMICOLON);
    *output = ast_loop_control_new();
    return true;
}

static AstNode *parser_next_full(Parser *parser, Token token) {
    switch (token.kind) {
        case TOKEN_IDENT:
        case TOKEN_INTEGER:
        case TOKEN_OPENING_CIRCLE_BRACE:
        case TOKEN_STRING:
        case TOKEN_OPENING_ANGLE_BRACE:
            parser_skip_next(parser);
            AstExpr *expr = parse_expr(parser);
            Token next = parser_take(parser);
            AstStmt *stmt = NULL;
            #define SASSIGN(kind) ast_stmt_new_short_assign(parser->mempool, expr, \
                NOT_NULL(parse_expr(parser)), ast_binop_kind_new(kind, next.slice))
            switch (next.kind) {
                case TOKEN_SEMICOLON:
                    return ast_node_new_stmt(parser->mempool, ast_stmt_new_expr(parser->mempool, expr));
                case TOKEN_APPEND: stmt = SASSIGN(AST_BINOP_ADD); break;
                case TOKEN_SUBTRACT: stmt = SASSIGN(AST_BINOP_SUBTRACT); break;
                case TOKEN_MULTIPLY: stmt = SASSIGN(AST_BINOP_MULTIPLY); break;
                case TOKEN_DIVIDE: stmt = SASSIGN(AST_BINOP_DIVIDE); break;
                case TOKEN_ASSIGN:
                    stmt = ast_stmt_new_assign(parser->mempool, expr, NOT_NULL(parse_expr(parser)));
                    break;
                default:
                    parser_skip_next(parser);
                    parser_err(parser, token.slice, "expected ';` or assign token");
                    return NULL;
            }
            assert(stmt);
            PARSER_EXPECT_NEXT(parser, TOKEN_SEMICOLON);
            return ast_node_new_stmt(parser->mempool, stmt);
        case TOKEN_FOR: {
            OptSlice label = opt_slice_new_null();
            if (parser_next_should_be(parser, TOKEN_DOT)) {
                label = opt_slice_new_value(PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice);
            }
            Slice name = PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice;
            PARSER_EXPECT_NEXT(parser, TOKEN_IN);
            AstExpr *expr = NOT_NULL(parse_expr(parser));
            AstBody *body = NOT_NULL(parse_body(parser));
            return ast_node_new_stmt(parser->mempool, ast_stmt_new_for(parser->mempool, name, expr, body, label));
        }
        case TOKEN_DO: {
            OptSlice label = opt_slice_new_null();
            if (parser_next_should_be(parser, TOKEN_DOT)) {
                label = opt_slice_new_value(PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice);
            }
            AstBody *body = NOT_NULL(parse_body(parser));
            PARSER_EXPECT_NEXT(parser, TOKEN_WHILE);
            AstExpr *cond = NOT_NULL(parse_expr(parser));
            PARSER_EXPECT_NEXT(parser, TOKEN_SEMICOLON);
            return ast_node_new_stmt(parser->mempool, ast_stmt_new_while(parser->mempool, cond, body, true, label));
        }
        case TOKEN_RETURN: {
            AstExpr *value = parser_next_is(parser, TOKEN_SEMICOLON) ? NULL : NOT_NULL(parse_expr(parser));
            PARSER_EXPECT_NEXT(parser, TOKEN_SEMICOLON);
            return ast_node_new_stmt(parser->mempool, ast_stmt_new_return(parser->mempool, token.slice, value));
        }
        case TOKEN_DEFER: {
            if (parser_next_is(parser, TOKEN_OPENING_FIGURE_BRACE)) {
                return ast_node_new_stmt(parser->mempool, ast_stmt_new_defer_body(parser->mempool,
                    NOT_NULL(parse_body(parser))));
            }
            AstExpr *expr = NOT_NULL(parse_expr(parser));
            PARSER_EXPECT_NEXT(parser, TOKEN_SEMICOLON);
            return ast_node_new_stmt(parser->mempool, ast_stmt_new_defer_expr(parser->mempool, expr));
        }
        case TOKEN_IF: return parse_if(parser);
        case TOKEN_WHILE: {
            OptSlice label = opt_slice_new_null();
            if (parser_next_should_be(parser, TOKEN_DOT)) {
                label = opt_slice_new_value(PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice);
            }
            AstExpr *cond = NOT_NULL(parse_expr(parser));
            AstBody *body = NOT_NULL(parse_body(parser));
            return ast_node_new_stmt(parser->mempool, ast_stmt_new_while(parser->mempool, cond, body, false, label));
        }
        case TOKEN_CONTINUE: {
            AstLoopControl output;
            NOT_NULL(parse_loop_control(parser, &output));
            return ast_node_new_stmt(parser->mempool, ast_stmt_new_continue(parser->mempool, output));
        }
        case TOKEN_BREAK: {
            AstLoopControl output;
            NOT_NULL(parse_loop_control(parser, &output));
            return ast_node_new_stmt(parser->mempool, ast_stmt_new_break(parser->mempool, output));
        }
        default: return parser_next_maybe_public(parser, token, false);
    }
}

AstNode *parser_next(Parser *parser) {
    while (true) {
        bool is_public = parser_next_should_be(parser, TOKEN_PUBLIC);
        Token token = parser_take(parser);
        if (token.kind == TOKEN_EOI) {
            return NULL;
        }
        AstNode *try = is_public ?
            parser_next_maybe_public(parser, token, true) :
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
