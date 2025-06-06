#pragma once

#include "ast/stmt.h"
#include "ir/api/stmt/stmt.h"
#include "sema/module/api/module.h"

IrStmt *sema_module_emit_stmt(SemaModule *module, AstNode *stmt);
