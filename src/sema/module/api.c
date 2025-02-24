#include "parser/api.h"
#include "ast/private/module.h"
#include "sema/module/impl.h"
#include "core/vec.h"

SemaModule *sema_module_new(SemaProject *project, AstModule *module) {
	SemaModule *result = malloc(sizeof(SemaModule));
	result->ast = module;
	result->public_decls = vec_new(SemaScopeDecl*);
    result->loops = vec_new(SemaLoop*);
	result->scopes = vec_new(SemaScope);
	result->failed = false;
    result->project = project;
	return result;
}

AstModule *sema_module_of(SemaModule *module) {
    return module->ast;
}

bool sema_module_failed(const SemaModule *sema) {
    return sema->failed;
}

struct SemaProject *sema_module_project(const SemaModule *sema) {
    return sema->project;
}
