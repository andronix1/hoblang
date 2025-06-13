#pragma once

#include "core/mempool.h"
#include "sema/module/api/decl.h"
#include "sema/module/api/value.h"

typedef struct SemaDecl {
    SemaModule *module;
    SemaValue *value;
} SemaDecl;

SemaDecl *sema_decl_new(Mempool *mempool, SemaModule *module, SemaValue *value);
