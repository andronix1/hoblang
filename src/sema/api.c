#include "api.h"
#include "core/mempool.h"
#include "sema/module/api.h"
#include "sema/project.h"
#include <stdlib.h>

SemaProject *sema_project_new(Path entry) {
    SemaProject *project = malloc(sizeof(SemaProject));
    project->mempool = mempool_new(1024);
    project->modules_map = keymap_new_in(project->mempool, SemaModule*);

    sema_project_read(project, entry);
    return project;
}

void sema_project_free(SemaProject *project) {
    for (size_t i = 0; i < vec_len(project->modules_map); i++) {
        keymap_at(project->modules_map, i, module);
        sema_module_free(module->value);
    }
    mempool_free(project->mempool);
    free(project);
}
