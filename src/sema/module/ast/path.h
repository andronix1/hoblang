#pragma once

#include "ast/api/path.h"
#include "sema/module/api/module.h"
#include "sema/module/api/value.h"

SemaValue *sema_module_path(SemaModule *module, AstPath *path);
SemaValue *sema_module_path_segment(SemaModule *module, AstPathSegment *path);
