#include "module.h"
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
    module->ss = NULL;
    module->mempool = mempool_new(1024);
    
    module->types = vec_new_in(module->mempool, SemaTypeInfo);
    module->local_decls_map = keymap_new_in(module->mempool, SemaDecl*);

    module->stage_failures = vec_new_in(module->mempool, bool);
    vec_resize(module->stage_failures, vec_len(nodes));
    memset(module->stage_failures, 0, sizeof(bool) * vec_len(nodes));

    return module;
}

void sema_module_run_stage(SemaModule *module, size_t stage_id) {
    assert(stage_id < sema_stages_count);
    for (size_t i = 0; i < vec_len(module->nodes); i++) {
        if (module->stage_failures[i]) continue;
        if (!sema_stages[stage_id](module, module->nodes[i])) {
            module->stage_failures[stage_id] = true;
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
