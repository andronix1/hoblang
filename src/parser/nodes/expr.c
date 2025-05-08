#include "expr.h"
#include "ast/api/expr.h"
#include "ast/expr.h"
#include "core/mempool.h"
#include "core/null.h"
#include "core/vec.h"
#include "lexer/token.h"
#include "parser/nodes/path.h"
#include "parser/parser.h"

static inline AstExpr *parse_middle_expr(Parser *parser) {
    Token token = parser_take(parser);
    switch (token.kind) {
        case TOKEN_OPENING_CIRCLE_BRACE: {
            AstExpr *expr = parse_expr(parser);
            PARSER_EXPECT_NEXT(parser, TOKEN_CLOSING_CIRCLE_BRACE);
            return expr;
        }
        case TOKEN_IDENT:
            parser_skip_next(parser);
            return ast_expr_new_path(parser->mempool, NOT_NULL(parse_path(parser)));
        case TOKEN_INTEGER:
            return ast_expr_new_integer(parser->mempool, token.integer);
        default:
            parser_err(parser, token.slice, "expected expression");
            return NULL;
    }
}

static inline AstExpr *parse_post_expr(Parser *parser, AstExpr *expr) {
    bool reading = true;
    while (reading) {
        Token token = parser_take(parser);
        switch (token.kind) {
            case TOKEN_OPENING_CIRCLE_BRACE: {
                AstExpr **args = vec_new_in(parser->mempool, AstExpr*);
                while (!parser_next_should_be(parser, TOKEN_CLOSING_CIRCLE_BRACE)) {
                    vec_push(args, parse_expr(parser));
                    if (!parser_check_list_sep(parser, TOKEN_CLOSING_CIRCLE_BRACE)) {
                        return NULL;
                    }
                }
                expr = ast_expr_new_callable(parser->mempool, expr, args);
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

AstExpr *parse_expr(Parser *parser) {
    return parse_post_expr(parser, NOT_NULL(parse_middle_expr(parser)));
}
