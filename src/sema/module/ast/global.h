#pragma once

#include "ast/global.h"
#include "core/opt_slice.h"

OptSlice sema_global_to_opt_slice(AstGlobal *global, Slice or);
