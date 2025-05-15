#include "body.h"
#include "core/vec.h"
#include "ast/body.h"
#include "llvm/module/nodes/node.h"

void llvm_emit_body(LlvmModule *module, AstBody *body) {
    for (size_t i = 0; i < vec_len(body->nodes); i++) {
        AstNode *node = body->nodes[i];
        llvm_module_read_node(module, node);
        llvm_module_emit_node(module, node);
    }
}
