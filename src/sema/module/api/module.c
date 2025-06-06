#include "module.h"
#include "core/log.h"
#include "core/mempool.h"
#include "core/vec.h"
#include "parser/api.h"
#include "sema/module/api/decl.h"
#include "sema/module/module.h"
#include "core/null.h"
#include "sema/module/stages/stages.h"
#include <stdlib.h>
#include <string.h>

SemaModule *sema_module_new(Ir *ir, Parser *parser) {
    AstNode **nodes = NOT_NULL(parser_parse(parser));
    SemaModule *module = malloc(sizeof(SemaModule));
    module->ir = ir;
    module->parser = parser;
    module->nodes = nodes;
    module->failed = false;
    module->mempool = mempool_new(1024);
    
    module->types = vec_new_in(module->mempool, SemaTypeInfo);
    module->local_decls_map = keymap_new_in(module->mempool, SemaDecl*);
    return module;
}

void sema_module_emit(SemaModule *module) {
    bool *stage_failures = vec_new_in(module->mempool, bool);
    vec_resize(stage_failures, vec_len(module->nodes));
    memset(stage_failures, 0, sizeof(bool) * vec_len(stage_failures));
    for (size_t i = 0; i < sema_stages_count; i++) {
        logln("running sema stage: $l", i);
        for (size_t j = 0; j < vec_len(module->nodes); j++) {
            if (stage_failures[j]) continue;
            if (!sema_stages[i](module, module->nodes[j])) {
                stage_failures[i] = true;
            }
        }
    }
}

bool sema_module_failed(SemaModule *module) {
    return module->failed;
}

void sema_module_free(SemaModule *module) {
    parser_free(module->parser);
    mempool_free(module->mempool);
    free(module);
}
