#pragma once

#include "ast/api/stmt.h"
#include "llvm/module/api.h"

void llvm_emit_stmt(LlvmModule *module, AstStmt *stmt);
