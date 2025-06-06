#pragma once

#include "ast/api/body.h"
#include "ir/api/stmt/code.h"
#include "sema/module/api/module.h"

IrCode *sema_module_emit_code(SemaModule *module, AstBody *body);
