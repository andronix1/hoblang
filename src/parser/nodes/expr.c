#include "expr.h"
#include "ast/api/expr.h"
#include "ast/expr.h"
#include "ast/path.h"
#include "ast/type.h"
#include "core/assert.h"
#include "core/keymap.h"
#include "core/math.h"
#include "core/mempool.h"
#include "core/null.h"
#include "core/slice.h"
#include "core/vec.h"
#include "lexer/token.h"
#include "parser/nodes/path.h"
#include "parser/nodes/type.h"
#include "parser/parser.h"
#include <stdio.h>

static inline AstExpr *parse_expr_additions(Parser *parser, AstExpr *expr) {
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
            case TOKEN_AS: {
                AstType *type = NOT_NULL(parse_type(parser));
                expr = ast_expr_new_as(parser->mempool, slice_union(expr->slice, type->slice), token.slice, expr, type);
                break;
            }
            case TOKEN_DOT: {
                AstPath *path = NOT_NULL(parse_path(parser));
                expr = ast_expr_new_inner_path(parser->mempool, slice_union(expr->slice, ast_path_slice(path)), expr, path);
                break;
            }
            default:
                parser_skip_next(parser);
                reading = false;
                break;
        }
    }
    return expr;
}

static inline AstExpr *_parse_middle_expr(Parser *parser) {
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
        case TOKEN_NOT: {
            AstExpr *inner = NOT_NULL(_parse_middle_expr(parser));
            return ast_expr_new_not(parser->mempool, slice_union(token.slice, inner->slice), inner);
        }
        case TOKEN_BITAND: {
            AstExpr *inner = NOT_NULL(_parse_middle_expr(parser));
            return ast_expr_new_take_ref(parser->mempool, slice_union(token.slice, inner->slice), inner);
        }
        case TOKEN_TRUE: return ast_expr_new_bool(parser->mempool, token.slice, true);
        case TOKEN_FALSE: return ast_expr_new_bool(parser->mempool, token.slice, false);
        case TOKEN_STRING:
            return ast_expr_new_string(parser->mempool, token.slice, token.string);
        case TOKEN_INTEGER:
            return ast_expr_new_integer(parser->mempool, token.slice, token.integer);
        case TOKEN_CHAR:
            return ast_expr_new_char(parser->mempool, token.slice, token.character);
        case TOKEN_OPENING_ANGLE_BRACE: {
            AstType *type = NOT_NULL(parse_type(parser));
            PARSER_EXPECT_NEXT(parser, TOKEN_CLOSING_ANGLE_BRACE);
            Token begin_token = parser_take(parser);
            switch (begin_token.kind) {
                case TOKEN_OPENING_FIGURE_BRACE: {
                    AstExprStructField *fields_map = keymap_new_in(parser->mempool, AstExprStructField);
                    while (parser_next_is_not(parser, TOKEN_CLOSING_FIGURE_BRACE)) {
                        Slice name = PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice;
                        PARSER_EXPECT_NEXT(parser, TOKEN_COLON);
                        AstExpr *expr = NOT_NULL(parse_expr(parser));
                        if (keymap_insert(fields_map, name, ast_expr_struct_field_new(expr))) {
                            parser_err(parser, name, "duplication of field `$S`", name);
                        }
                        if (!parser_check_list_sep(parser, TOKEN_CLOSING_FIGURE_BRACE)) return false;
                    }
                    Token closing = PARSER_EXPECT_NEXT(parser, TOKEN_CLOSING_FIGURE_BRACE);
                    return ast_expr_new_struct(parser->mempool,
                        slice_union(token.slice, closing.slice), type, fields_map);
                }
                default:
                    parser_err(parser, begin_token.slice, "expected `{`");
                    return NULL;
            }
            return ast_expr_new_integer(parser->mempool, token.slice, token.integer);
        }
        default:
            parser_err(parser, token.slice, "expected expression");
            return NULL;
    }
}

static AstExpr *parse_middle_expr(Parser *parser) {
    return parse_expr_additions(parser, NOT_NULL(_parse_middle_expr(parser)));
}

static int get_binop_kind_priority(AstBinopKindKind kind) {
    switch (kind) {
        case AST_BINOP_OR: return -100;
        case AST_BINOP_AND: return -100;
        case AST_BINOP_EQUALS: return -10;
        case AST_BINOP_NOT_EQUALS: return -10;
        case AST_BINOP_LESS: return 0;
        case AST_BINOP_GREATER: return 0;
        case AST_BINOP_LESS_EQ: return 0;
        case AST_BINOP_GREATER_EQ: return 0;
        case AST_BINOP_ADD: return 10;
        case AST_BINOP_SUBTRACT: return 10;
        case AST_BINOP_MOD: return 100;
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
    int root_priority = get_binop_kind_priority(expr->binop.kind.kind);
    int left_priority = get_binop_kind_priority(left->binop.kind.kind);
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

static inline AstExpr *create_expr_lprioritized(Parser *parser, Slice slice, AstBinopKindKind kind, AstExpr *left) {
    AstExpr *right = NOT_NULL(parse_middle_expr(parser));
    AstExpr *result = ast_expr_new_binop(parser->mempool, slice_union(left->slice, right->slice),
        ast_binop_kind_new(kind, slice), left, right);
    swap_binop_prioritized(result);
    return result;
}

static inline AstExpr *parse_post_expr(Parser *parser, AstExpr *expr) {
    bool reading = true;
    while (reading) {
        Token token = parser_take(parser);
        switch (token.kind) {
            case TOKEN_OR: expr = NOT_NULL(create_expr_lprioritized(parser, token.slice, AST_BINOP_OR, expr)); break;
            case TOKEN_AND: expr = NOT_NULL(create_expr_lprioritized(parser, token.slice, AST_BINOP_AND, expr)); break;
            case TOKEN_PLUS: expr = NOT_NULL(create_expr_lprioritized(parser, token.slice, AST_BINOP_ADD, expr)); break;
            case TOKEN_MINUS: expr = NOT_NULL(create_expr_lprioritized(parser, token.slice, AST_BINOP_SUBTRACT, expr)); break;
            case TOKEN_MOD: expr = NOT_NULL(create_expr_lprioritized(parser, token.slice, AST_BINOP_MOD, expr)); break;
            case TOKEN_STAR: expr = NOT_NULL(create_expr_lprioritized(parser, token.slice, AST_BINOP_MULTIPLY, expr)); break;
            case TOKEN_SLASH: expr = NOT_NULL(create_expr_lprioritized(parser, token.slice, AST_BINOP_DIVIDE, expr)); break;
            case TOKEN_EQUALS: expr = NOT_NULL(create_expr_lprioritized(parser, token.slice, AST_BINOP_EQUALS, expr)); break;
            case TOKEN_NOT_EQUALS: expr = NOT_NULL(create_expr_lprioritized(parser, token.slice, AST_BINOP_NOT_EQUALS, expr)); break;
            case TOKEN_OPENING_ANGLE_BRACE: expr = NOT_NULL(create_expr_lprioritized(parser, token.slice, AST_BINOP_LESS, expr)); break;
            case TOKEN_CLOSING_ANGLE_BRACE: expr = NOT_NULL(create_expr_lprioritized(parser, token.slice, AST_BINOP_GREATER, expr)); break;
            case TOKEN_LESS_EQ: expr = NOT_NULL(create_expr_lprioritized(parser, token.slice, AST_BINOP_LESS_EQ, expr)); break;
            case TOKEN_GREATER_EQ: expr = NOT_NULL(create_expr_lprioritized(parser, token.slice, AST_BINOP_GREATER_EQ, expr)); break;
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
