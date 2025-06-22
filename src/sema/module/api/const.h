#pragma once

#include "core/mempool.h"
#include "hir/api/const.h"
#include "sema/module/api/type.h"

typedef struct SemaConst SemaConst;

HirConst sema_const_to_hir(SemaModule *module, SemaConst *constant);
SemaConst *sema_const_nest(Mempool *mempool, SemaConst *constant, SemaType *type);
