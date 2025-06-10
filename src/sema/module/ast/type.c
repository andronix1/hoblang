#include "type.h"
#include "ast/type.h"
#include "core/assert.h"
#include "core/null.h"
#include "sema/module/api/value.h"
#include "sema/module/ast/path.h"
#include "sema/module/type.h"

SemaType *sema_module_type(SemaModule *module, AstType *type) {
    switch (type->kind) {
        case AST_TYPE_PATH: {
            return NOT_NULL(sema_value_should_be_type(module, type->slice,
                NOT_NULL(sema_module_path(module, type->path))));
        }
        case AST_TYPE_POINTER:
            return sema_type_new_pointer(module, NOT_NULL(sema_module_type(module, type->pointer_to)));
        case AST_TYPE_STRUCT:
        case AST_TYPE_SLICE:
            TODO;
    }
    UNREACHABLE;
}
