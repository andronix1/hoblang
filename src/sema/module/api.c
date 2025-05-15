#include "api.h"
#include "parser/api.h"
#include "sema/module/module.h"
#include "sema/module/nodes/node.h"
#include <stdlib.h>

SemaModule *sema_module_new(Parser *parser) {
    SemaModule *module = malloc(sizeof(SemaModule));
    module->parser = parser;
    module->nodes = NULL;
    module->mempool = mempool_new(1024);
    module->decls_map = keymap_new_in(module->mempool, SemaDecl);
    module->ss = NULL;
    return module;
}

void sema_module_free(SemaModule *module) {
    mempool_free(module->mempool);
    parser_free(module->parser);
    free(module);
}

bool sema_module_read(SemaModule *module) {
    assert(!module->nodes);
    module->nodes = parser_parse(module->parser);
    if (!module->nodes) {
        return false;
    }
    sema_module_push_primitives(module);
    for (size_t i = 0; i < vec_len(module->nodes); i++) {
        sema_module_read_node(module, module->nodes[i]);
    }
    return true;
}

AstNode **sema_module_nodes(SemaModule *module) {
    assert(module->nodes);
    return module->nodes;
}

void sema_module_analyze(SemaModule *module) {
    for (size_t i = 0; i < vec_len(module->nodes); i++) {
        sema_module_analyze_node(module, module->nodes[i]);
    }
}
