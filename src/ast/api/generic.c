#include "generic.h"
#include "ast/generic.h"
#include "ast/type.h"
#include "core/vec.h"

bool ast_generic_builder_eq(const AstGenericBuilder *a, const AstGenericBuilder *b) {
    if (vec_len(a->params) != vec_len(b->params)) return false;
    for (size_t i = 0; i < vec_len(a->params); i++) {
        if (!ast_type_eq(a->params[i], b->params[i])) return false;
    }
    return true;
}
