#include "expr.h"
#include "ast/expr.h"
#include "core/null.h"
#include "lexer/token.h"
#include "parser/nodes/path.h"
#include "parser/parser.h"

AstExpr *parse_expr(Parser *parser) {
    Token token = parser_take(parser);
    switch (token.kind) {
        case TOKEN_IDENT:
            parser_skip_next(parser);
            return ast_expr_new_path(parser->mempool, NOT_NULL(parse_path(parser)));
        default:
            parser_err(parser, token.slice, "expected expression");
            return NULL;
    }
}
