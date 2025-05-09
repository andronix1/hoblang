#pragma once

#include "ast/api/node.h"
#include "lexer/api.h"

typedef struct Parser Parser;

Parser *parser_new(Lexer *lexer);
AstNode *parser_next(Parser *parser);
AstNode **parser_parse(Parser *parser);
void parser_free(Parser *parser);
