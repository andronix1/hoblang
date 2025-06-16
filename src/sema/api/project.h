#pragma once

#include "sema/module/api/module.h"

typedef struct SemaProject SemaProject;

SemaProject *sema_project_new(Ir *ir, Path *lib_dirs);
SemaModule *sema_project_add_module(SemaProject *project, const Path from, const Path path, bool force_internal);
SemaModule *sema_project_add_library(SemaProject *project, Slice name);
SemaModule *sema_project_add_internal_library(SemaProject *project, Slice name);
void sema_project_emit(SemaProject *project);
bool sema_project_failed(SemaProject *project);
void sema_project_free(SemaProject *project);
