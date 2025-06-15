#pragma once

#include "core/mempool.h"
#include "ir/api/const.h"
#include "sema/module/api/type.h"

typedef struct SemaConst SemaConst;

IrConst *sema_const_to_ir(Mempool *mempool, SemaConst *constant);
SemaConst *sema_const_nest(Mempool *mempool, SemaConst *constant, SemaType *type);
