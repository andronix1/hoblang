#include "node.h"
#include "mempool.h"

#define FIELD(FROM, TO) out->FROM = TO;

#define CONSTRUCT(KIND, FIELDS) MEMPOOL_CONSTRUCT(AstNode, { \
    out->kind = KIND; \
    FIELDS; \
})

AstNode *ast_node_new_type_decl(Mempool *mempool, Slice name, AstType *type)
    CONSTRUCT(AST_NODE_TYPE_DECL,
        FIELD(decl.name, name)
        FIELD(decl.type, type))
