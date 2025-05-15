#pragma once

#include "ast/api/body.h"
#include "llvm/module/api.h"

void llvm_emit_body(LlvmModule *module, AstBody *body);
