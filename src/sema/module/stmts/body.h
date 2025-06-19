#pragma once

#include "ast/api/body.h"
#include "sema/module/api/module.h"
#include "sema/module/scope.h"

HirCode *sema_module_emit_code(SemaModule *module, AstBody *body, SemaLoop *loop);
