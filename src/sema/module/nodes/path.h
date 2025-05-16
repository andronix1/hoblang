#pragma once

#include "ast/api/path.h"
#include "sema/module/api.h"
#include "sema/module/api/value.h"

void sema_path_ignore_before(AstPath *path, size_t before);
SemaValue *sema_module_resolve_path(SemaModule *module, AstPath *path);
