#include "api.h"

#include <malloc.h>
#include "core/mempool.h"
#include "lexer/api.h"
#include "parser.h"

Parser *parser_new(Lexer *lexer, bool lexer_ownership) {
    Parser *parser = malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->lexer_ownership = lexer_ownership;
    parser->mempool = mempool_new(1024);
    return parser;
}

void parser_free(Parser *parser) {
    if (parser->lexer_ownership) {
        lexer_free(parser->lexer);
    }
    mempool_free(parser->mempool);
    free(parser);
}
