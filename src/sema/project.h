#pragma once

#include "core/mempool.h"
#include "hir/api/hir.h"
#include "sema/module/api/module.h"

typedef struct SemaProject {
    SemaModule **modules_map;
    Mempool *mempool;
    Hir *hir;

    Path *lib_dirs;
} SemaProject;

