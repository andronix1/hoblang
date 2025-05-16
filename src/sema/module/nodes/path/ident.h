#pragma once

#include "ast/api/path.h"
#include "sema/module/api.h"
#include "sema/module/api/value.h"

SemaValue *sema_path_resolve_ident(SemaModule *module, SemaValue *value, AstPath *path, size_t idx);
