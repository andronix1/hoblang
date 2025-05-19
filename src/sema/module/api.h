#pragma once

#include "parser/api.h"
#include "sema/api.h"

typedef struct SemaModule SemaModule;

SemaModule *sema_module_new(Parser *parser, SemaProject *project);
bool sema_module_read(SemaModule *module);
void sema_module_analyze(SemaModule *module);
AstNode **sema_module_nodes(SemaModule *module);
void sema_module_free(SemaModule *module);
