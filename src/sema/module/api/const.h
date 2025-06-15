#pragma once

#include "core/mempool.h"
#include "ir/api/const.h"

typedef struct SemaConst SemaConst;

IrConst *sema_const_to_ir(Mempool *mempool, SemaConst *constant);
