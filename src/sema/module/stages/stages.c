#include "stages.h"
#include "sema/module/stages/emit_bodies.h"
#include "sema/module/stages/fill_decls.h"
#include "sema/module/stages/init_decls.h"

#define SEMA_STAGES_COUNT 3

size_t sema_stages_count = SEMA_STAGES_COUNT;
size_t sema_setup_stages_count = 2;

SemaStage sema_stages[SEMA_STAGES_COUNT] = {
    sema_module_init_node_decls,
    sema_module_fill_node_decls,
    sema_module_emit_node_body,
};
