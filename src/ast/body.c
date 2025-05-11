#include "body.h"
#include "core/mempool.h"
#include "ast/node.h"
#include "core/vec.h"

bool ast_body_eq(const AstBody *a, const AstBody *b) {
    if (vec_len(a->nodes) != vec_len(b->nodes)) {
        return false;
    }
    for (size_t i = 0; i < vec_len(a->nodes); i++) {
        if (!ast_node_eq(a->nodes[i], b->nodes[i])) {
            return false;
        }
    }
    return true;
}

AstBody *ast_body_new(Mempool *mempool, AstNode **nodes)
    MEMPOOL_CONSTRUCT(AstBody, out->nodes = nodes)
