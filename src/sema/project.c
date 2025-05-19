#include "project.h"
#include "core/file_content.h"
#include "core/keymap.h"
#include "core/log.h"
#include "core/mempool.h"
#include "core/path.h"
#include "core/vec.h"
#include "lexer/api.h"
#include "parser/api.h"
#include "sema/module/api.h"

SemaModule *sema_project_read(SemaProject *project, const Path path) {
    Path real_path_cstr = path_realpath_in(project->mempool, path);
    Slice real_path = slice_from_cstr(real_path_cstr);
    SemaModule **module_ref = keymap_get(project->modules_map, real_path);
    if (module_ref) {
        return *module_ref;
    }
    FileContent *content = file_content_read(real_path_cstr);
    if (!content) {
        logln("error: failed to open file $s", real_path_cstr);
        return NULL;
    }
    SemaModule *module = sema_module_new(parser_new(lexer_new(content)), project);
    if (!sema_module_read(module)) {
        return NULL;
    }
    keymap_insert(project->modules_map, real_path, module);
    return module;
}

SemaModule **sema_project_modules(SemaProject *project) {
    SemaModule **modules = vec_new_in(project->mempool, SemaModule*);
    for (size_t i = 0; i < vec_len(project->modules_map); i++) {
        keymap_at(project->modules_map, i, module);
        vec_push(modules, module->value);
    }
    return modules;
}

bool sema_project_analyze(SemaProject *project) {
    for (size_t i = 0; i < vec_len(project->modules_map); i++) {
        keymap_at(project->modules_map, i, module);
        sema_module_analyze(module->value);
    }
    return true;
}
