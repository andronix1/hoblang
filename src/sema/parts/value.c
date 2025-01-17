#include "../parts.h"

SemaType *sema_ast_value(SemaModule *sema, AstValue *value) {
	SemaValueDecl *decl = sema_resolve_value_path(sema, &value->mod_path);
	if (!decl) {
		return NULL;
	}	
	SemaType *type = decl->type;
	value->sema_type = type;
	for (size_t i = 0; i < vec_len(value->segments); i++) {
		AstValueSegment *seg = &value->segments[i];
		switch (seg->type) {
			case AST_VALUE_IDENT:
				sema_err("inner idents are not supported now");
				return NULL;

			case AST_VALUE_IDX: {
				if (type->type != SEMA_TYPE_POINTER) {
					sema_err("only pointers can be indexed");
					return NULL;
				}
				type = type->ptr_to;
				SemaType *idx_type = sema_ast_expr_type(sema, seg->idx, &primitives[PRIMITIVE_I32]);
				if (idx_type && !sema_types_equals(idx_type, &primitives[PRIMITIVE_I32])) {
					sema_err("indexes must be i32");
				}
				break;
			}

			case AST_VALUE_DEREF:
				if (type->type != SEMA_TYPE_POINTER) {
					sema_err("only pointers can be dereferenced");
					return NULL;
				}
				type = type->ptr_to;
				break;
		}
		value->sema_type = seg->sema_type = type;
	}
	return type;
}
