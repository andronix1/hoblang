#pragma once

#include "ast/api/node.h"
#include "sema/module/api/module.h"

typedef bool (*SemaStage)(SemaModule *module, AstNode *node);

extern size_t sema_stages_count;
extern SemaStage sema_stages[];
