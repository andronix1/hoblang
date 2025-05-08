#pragma once

#include "ast/api/node.h"
#include "core/mempool.h"
#include <stdbool.h>

typedef struct AstBody {
    AstNode **nodes;
} AstBody;

bool ast_body_eq(const AstBody *a, const AstBody *b);

AstBody *ast_body_new(Mempool *mempool, AstNode **nodes);
