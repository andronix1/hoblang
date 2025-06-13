#pragma once

#include "sema/module/api/module.h"

typedef struct SemaProject SemaProject;

SemaProject *sema_project_new(Ir *ir);
SemaModule *sema_project_add_module(SemaProject *project, const Path from, const Path path);
void sema_project_emit(SemaProject *project);
bool sema_project_failed(SemaProject *project);
void sema_project_free(SemaProject *project);
