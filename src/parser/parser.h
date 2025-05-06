#pragma once

#include "core/mempool.h"
#include "lexer/api.h"

typedef struct Parser {
    Mempool *mempool;
    Lexer *lexer;
    bool lexer_ownership;
} Parser;
