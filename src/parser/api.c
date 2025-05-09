#include "api.h"

#include <malloc.h>
#include "core/mempool.h"
#include "lexer/api.h"
#include "parser.h"

Parser *parser_new(Lexer *lexer) {
    Parser *parser = malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->mempool = mempool_new(1024);
    parser->skip_next = false;
    return parser;
}

void parser_free(Parser *parser) {
    lexer_free(parser->lexer);
    mempool_free(parser->mempool);
    free(parser);
}
