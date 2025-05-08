#include "node.h"
#include "ast/type.h"
#include "core/assert.h"
#include "core/slice.h"
#include "mempool.h"

#define FIELD(FROM, TO) out->FROM = TO;

#define CONSTRUCT(KIND, FIELDS) MEMPOOL_CONSTRUCT(AstNode, { \
    out->kind = KIND; \
    FIELDS; \
})

bool ast_node_eq(const AstNode *a, const AstNode *b) {
    if (a->kind != b->kind) {
        return false;
    }
    switch (a->kind) {
        case AST_NODE_TYPE_DECL:
            return a->type_decl.is_local == b->type_decl.is_local &&
                slice_eq(a->type_decl.name, b->type_decl.name) &&
                ast_type_eq(a->type_decl.type, b->type_decl.type);
    }
    UNREACHABLE;
}


AstNode *ast_node_new_type_decl(Mempool *mempool, bool is_local, Slice name, AstType *type)
    CONSTRUCT(AST_NODE_TYPE_DECL,
        FIELD(type_decl.name, name)
        FIELD(type_decl.type, type)
        FIELD(type_decl.is_local, is_local))
