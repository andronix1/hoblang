#pragma once

#include "ast/api/path.h"
#include "sema/module/api.h"
#include "sema/module/api/value.h"

SemaValue *sema_module_resolve_path(SemaModule *module, AstPath *path);
