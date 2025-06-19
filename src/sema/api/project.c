#include "project.h"
#include "sema/module/module.h"
#include "sema/module/stages/setup.h"
#include "sema/project.h"
#include "core/file_content.h"
#include "core/keymap.h"
#include "core/log.h"
#include "core/mempool.h"
#include "core/path.h"
#include "core/vec.h"
#include "lexer/api.h"
#include "parser/api.h"
#include "sema/module/api/module.h"
#include "sema/module/stages/stages.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

SemaProject *sema_project_new(Hir *ir, Path *lib_dirs) {
    SemaProject *project = malloc(sizeof(SemaProject));
    project->mempool = mempool_new(1024);
    project->modules_map = keymap_new_in(project->mempool, SemaModule*);
    project->hir = ir;
    project->lib_dirs = lib_dirs;
    return project;
}

inline SemaModule *sema_project_add_module(SemaProject *project, Path from, Path path, bool force_internal) {
    if (from) {
        path = path_join_in(project->mempool, path_dirname_in(project->mempool, from), path);
    }
    Path new_path = path_realpath_in(project->mempool, path);
    if (!new_path) {
        logln("error: failed to load module at `$s`", path);
        return NULL;
    }
    path = new_path;
    Slice path_slice = slice_from_cstr(path);
    SemaModule **module = keymap_get(project->modules_map, path_slice);
    if (module) {
        return *module;
    }
    FileContent *content = file_content_read(path);
    if (!content) {
        logln("failed to read file `$S`", path);
        return NULL;
    }
    SemaModule *new_module = sema_module_new(project->hir, parser_new(lexer_new(content)));
    keymap_insert(project->modules_map, path_slice, new_module);
    sema_module_link_project(new_module, project);
    if (force_internal) {
        sema_module_make_no_std(new_module);
    }
    sema_module_setup(new_module);
    return new_module;
}

static inline SemaModule *_sema_project_add_library(SemaProject *project, Slice name, bool force_internal) {
    for (size_t i = 0; i < vec_len(project->lib_dirs); i++) {
        Path dir = path_join_in(project->mempool, project->lib_dirs[i], mempool_slice_to_cstr(project->mempool, name));
        Path entry = path_join_in(project->mempool, dir, "lib.hob");
        if (!access(entry, F_OK)) {
            return sema_project_add_module(project, NULL, entry, force_internal);
        }
    }
    logln("failed to find library `$S`", name);
    return NULL;
}

SemaModule *sema_project_add_library(SemaProject *project, Slice name) {
    return _sema_project_add_library(project, name, false);
}

SemaModule *sema_project_add_internal_library(SemaProject *project, Slice name) {
    return _sema_project_add_library(project, name, true);
}

void sema_project_emit(SemaProject *project) {
    for (size_t stage_id = sema_setup_stages_count; stage_id < sema_stages_count; stage_id++) {
        for (size_t i = 0; i < vec_len(project->modules_map); i++) {
            keymap_at(project->modules_map, i, module);
            sema_module_run_stage(module->value, stage_id);
        }
    }
}

bool sema_project_failed(SemaProject *project) {
    for (size_t i = 0; i < vec_len(project->modules_map); i++) {
        keymap_at(project->modules_map, i, module);
        if (sema_module_failed(module->value)) {
            return true;
        }
    }
    return false;
}

void sema_project_free(SemaProject *project) {
    for (size_t i = 0; i < vec_len(project->modules_map); i++) {
        keymap_at(project->modules_map, i, module);
        sema_module_free(module->value);
    }
    mempool_free(project->mempool);
    free(project);
}
