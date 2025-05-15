#pragma once

#include "parser/api.h"

typedef struct SemaModule SemaModule;

SemaModule *sema_module_new(Parser *parser);
bool sema_module_read(SemaModule *module);
void sema_module_analyze(SemaModule *module);
AstNode **sema_module_nodes(SemaModule *module);
void sema_module_free(SemaModule *module);
