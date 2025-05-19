#pragma once

#include "core/mempool.h"
#include "sema/module/api.h"

typedef struct SemaProject {
    Mempool *mempool;
    SemaModule **modules_map;
} SemaProject;

SemaModule *sema_project_read(SemaProject *project, const Path path);
SemaModule **sema_project_modules(SemaProject *project);
bool sema_project_analyze(SemaProject *project);
