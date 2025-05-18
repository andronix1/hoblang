#include "if.h"
#include "ast/node.h"
#include "ast/stmt.h"
#include "core/mempool.h"
#include "core/null.h"
#include "lexer/token.h"
#include "parser/nodes/body.h"
#include "parser/nodes/expr.h"
#include "parser/parser.h"

bool parse_cond_block(Parser *parser, AstCondBlock **conds) {
    AstExpr *expr = NOT_NULL(parse_expr(parser));
    AstBody *body = NOT_NULL(parse_body(parser));
    vec_push(*conds, ast_cond_block_new(expr, body));
    return true;
}

AstNode *parse_if(Parser *parser) {
    AstCondBlock *conds = vec_new_in(parser->mempool, AstCondBlock);
    NOT_NULL(parse_cond_block(parser, &conds));
    AstBody *else_body = NULL;
    while (parser_next_should_be(parser, TOKEN_ELSE)) {
        if (parser_next_should_be(parser, TOKEN_IF)) {
            NOT_NULL(parse_cond_block(parser, &conds));
        } else {
            else_body = NOT_NULL(parse_body(parser));
            break;
        }
    }
    return ast_node_new_stmt(parser->mempool, ast_stmt_new_if(parser->mempool, conds, else_body));
}
