#pragma once

#include "lexer/api.h"

typedef struct Parser Parser;

Parser *parser_new(Lexer *lexer, bool lexer_ownership);
void parser_free(Parser *parser);
