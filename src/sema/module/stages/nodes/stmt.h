#pragma once

#include "ast/stmt.h"
#include "sema/module/api/module.h"

bool sema_module_stage_init_stmt(SemaModule *module, Slice slice);
