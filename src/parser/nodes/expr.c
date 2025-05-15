#include "expr.h"
#include "ast/api/expr.h"
#include "ast/expr.h"
#include "ast/path.h"
#include "core/assert.h"
#include "core/math.h"
#include "core/mempool.h"
#include "core/null.h"
#include "core/slice.h"
#include "core/vec.h"
#include "lexer/token.h"
#include "parser/nodes/path.h"
#include "parser/parser.h"
#include <stdio.h>

static inline AstExpr *parse_middle_expr(Parser *parser) {
    Token token = parser_take(parser);
    switch (token.kind) {
        case TOKEN_OPENING_CIRCLE_BRACE: {
            AstExpr *expr = parse_expr(parser);
            Slice slice = PARSER_EXPECT_NEXT(parser, TOKEN_CLOSING_CIRCLE_BRACE).slice;
            return ast_expr_new_scope(parser->mempool, slice_union(token.slice, slice), expr);
        }
        case TOKEN_IDENT: {
            parser_skip_next(parser);
            AstPath *path = NOT_NULL(parse_path(parser));
            return ast_expr_new_path(parser->mempool, ast_path_slice(path), path);
        }
        case TOKEN_STRING:
            return ast_expr_new_string(parser->mempool, token.slice, token.string);
        case TOKEN_INTEGER:
            return ast_expr_new_integer(parser->mempool, token.slice, token.integer);
        default:
            parser_err(parser, token.slice, "expected expression");
            return NULL;
    }
}

static int get_binop_kind_priority(AstBinopKind kind) {
    switch (kind) {
        case AST_BINOP_ADD: return 10;
        case AST_BINOP_SUBTRACT: return 10;
        case AST_BINOP_MULTIPLY: return 100;
        case AST_BINOP_DIVIDE: return 100;
    }
    UNREACHABLE;
}

static inline void swap_binop_prioritized(AstExpr *expr) {
    assert(expr->kind == AST_EXPR_BINOP);
    AstExpr *left = expr->binop.left;
    if (left->kind != AST_EXPR_BINOP) {
        return;
    }
    int root_priority = get_binop_kind_priority(expr->binop.kind);
    int left_priority = get_binop_kind_priority(left->binop.kind);
    if (left_priority >= root_priority) {
        return;
    }
    swap(expr->binop.kind, left->binop.kind);
    AstExpr *ll = left->binop.left;
    AstExpr *lr = left->binop.right;
    AstExpr *r = expr->binop.right;
    expr->binop.left = ll;
    left->binop.left = lr;
    left->binop.right = r;
    expr->binop.right = left;
    swap_binop_prioritized(left);
}

static inline AstExpr *create_expr_lprioritized(Parser *parser, AstBinopKind kind, AstExpr *left) {
    AstExpr *right = NOT_NULL(parse_middle_expr(parser));
    AstExpr *result = ast_expr_new_binop(parser->mempool, slice_union(left->slice, right->slice), kind, left, right);
    swap_binop_prioritized(result);
    return result;
}

static inline AstExpr *parse_post_expr(Parser *parser, AstExpr *expr) {
    bool reading = true;
    while (reading) {
        Token token = parser_take(parser);
        switch (token.kind) {
            case TOKEN_OPENING_CIRCLE_BRACE: {
                AstExpr **args = vec_new_in(parser->mempool, AstExpr*);
                while (parser_next_is_not(parser, TOKEN_CLOSING_CIRCLE_BRACE)) {
                    vec_push(args, parse_expr(parser));
                    if (!parser_check_list_sep(parser, TOKEN_CLOSING_CIRCLE_BRACE)) {
                        return NULL;
                    }
                }
                Slice slice = PARSER_EXPECT_NEXT(parser, TOKEN_CLOSING_CIRCLE_BRACE).slice;
                expr = ast_expr_new_callable(parser->mempool, slice_union(expr->slice, slice), expr, args);
                break;
            }
            case TOKEN_PLUS:
                expr = NOT_NULL(create_expr_lprioritized(parser, AST_BINOP_ADD, expr));
                break;
            case TOKEN_MINUS:
                expr = NOT_NULL(create_expr_lprioritized(parser, AST_BINOP_SUBTRACT, expr));
                break;
            case TOKEN_STAR:
                expr = NOT_NULL(create_expr_lprioritized(parser, AST_BINOP_MULTIPLY, expr));
                break;
            case TOKEN_SLASH:
                expr = NOT_NULL(create_expr_lprioritized(parser, AST_BINOP_DIVIDE, expr));
                break;
            default:
                parser_skip_next(parser);
                reading = false;
                break;
        }
    }
    return expr;
}

AstExpr *parse_expr(Parser *parser) {
    return parse_post_expr(parser, NOT_NULL(parse_middle_expr(parser)));
}
