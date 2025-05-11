#pragma once

#include "ast/api/node.h"
#include "sema/module/api.h"

void sema_module_read_node(SemaModule *module, AstNode *node);
bool sema_module_analyze_node(SemaModule *module, AstNode *node);
