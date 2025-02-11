#pragma once

#include "ast/api.h"

typedef struct SemaModule SemaModule;

// TODO: API
typedef struct SemaProject SemaProject;

SemaModule *sema_module_new(SemaProject *project, AstModule *module);
AstModule *sema_module_of(SemaModule *module);
void sema_module_read_decls(SemaModule *sema);
void sema_module_analyze(SemaModule *sema);
bool sema_module_failed(const SemaModule *sema);

SemaProject *sema_module_project(const SemaModule *sema);
