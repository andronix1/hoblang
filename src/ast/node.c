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
            return slice_eq(a->decl.name, b->decl.name) && ast_type_eq(a->decl.type, b->decl.type);
    }
    UNREACHABLE;
}


AstNode *ast_node_new_type_decl(Mempool *mempool, Slice name, AstType *type)
    CONSTRUCT(AST_NODE_TYPE_DECL,
        FIELD(decl.name, name)
        FIELD(decl.type, type))
