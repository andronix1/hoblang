#pragma once

#include "ast/node.h"
#include "sema/module/api/module.h"

bool sema_module_stage_init_use(SemaModule *module, AstUse *use);
