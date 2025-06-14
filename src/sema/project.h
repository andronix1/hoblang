#pragma once

#include "core/mempool.h"
#include "sema/module/api/module.h"

typedef struct SemaProject {
    SemaModule **modules_map;
    Mempool *mempool;
    Ir *ir;

    Path libraries_path;
} SemaProject;

