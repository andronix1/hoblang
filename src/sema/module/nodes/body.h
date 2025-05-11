#pragma once

#include "ast/api/body.h"
#include "sema/module/api.h"

bool sema_module_analyze_body(SemaModule *module, AstBody *body);
