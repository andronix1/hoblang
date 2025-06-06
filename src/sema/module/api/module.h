#pragma once

#include "ir/api/ir.h"
#include "parser/api.h"

typedef struct SemaModule SemaModule;

SemaModule *sema_module_new(Ir *ir, Parser *parser);
void sema_module_emit(SemaModule *module);
bool sema_module_failed(SemaModule *module);
void sema_module_free(SemaModule *module);
