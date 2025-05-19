#pragma once

#include "core/path.h"

typedef struct SemaProject SemaProject;

SemaProject *sema_project_new(Path entry);
void sema_project_free(SemaProject *project);
