#include "api.h"
#include "core/mempool.h"
#include "core/vec.h"
#include "lexer/token.h"
#include "parser/parser.h"

static AstNode *parser_next(Parser *parser) {
    while (true) {
        Token token = parser_take(parser);
        switch (token.kind) {
            case TOKEN_TYPE: {
                parser_err(parser, token.slice, "type parsing is NIY");
                break;
            }
            case TOKEN_EOI: {
                return NULL;
            }
            default:
                parser_err(parser, token.slice, "unexpected token $T", token);
                break;
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
