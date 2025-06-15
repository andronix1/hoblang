#include "import.h"
#include "core/assert.h"
#include "core/null.h"
#include "sema/api/project.h"
#include "sema/module/decl.h"
#include "sema/module/module.h"
#include "sema/module/value.h"

bool sema_module_stage_init_import(SemaModule *module, AstImport *import) {
    switch (import->kind) {
        case AST_IMPORT_MODULE: {
            SemaModule *imported = NOT_NULL(sema_project_add_module(module->project, sema_module_file_path(module),
                mempool_slice_to_cstr(module->mempool, import->module.path)));
            if (!import->has_alias) {
                sema_module_err(module, import->module.path_slice, "import alias must be specified for importing module");
                return false;
            }
            sema_module_push_decl(module, import->alias, sema_decl_new(module->mempool,
                import->is_public ? NULL : module,
                sema_value_new_module(module->mempool, imported)));
            return true;
        }
        case AST_IMPORT_LIBRARY: {
            SemaModule *imported = NOT_NULL(sema_project_add_library(module->project, import->library.name));
            sema_module_push_decl(module,
                import->has_alias ? import->alias : import->library.name,
                sema_decl_new(module->mempool, import->is_public ? NULL : module,
                    sema_value_new_module(module->mempool, imported)));
            return true;
        }
    }
    UNREACHABLE;
}
