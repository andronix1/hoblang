#include "ast/private/path.h"
#include "ast/private/type.h"
#include "ast/private/module_node.h"
#include "core/vec.h"
#include "sema/type/private.h"
#include "sema/module/private.h"
#include "sema/module/decls/impl.h"
#include "sema/module/parts/decls/struct/api.h"
#include "sema/module/parts/decls/struct/impl.h"
#include "sema/value/private.h"

SemaScopeDecl *sema_resolve_decl_path_raw(SemaModule *sema, AstDeclPath *path) {
	SemaModule *module = sema;
	SemaScopeDecl *decl = NULL;
	for (size_t i = 0; i < vec_len(path->segments); i++) {
		decl = (i == 0 ?
				sema_module_resolve_scope_decl :
				sema_module_resolve_public_decl
			)(module, &path->segments[i]);
        if (!decl) {
            sema_err("`{slice}` in `{ast::dpath}` was not defined", &path->segments[i], path);
            return NULL;
        }
		if (i + 1 != vec_len(path->segments)) {
			if (decl->type != SEMA_SCOPE_DECL_MODULE) {
				sema_err("`{slice}` in `{ast::dpath}` is not a module", &path->segments[i], path);
				return NULL;
			}
			module = decl->module;
		}
	}
    path->decl = decl;
	assert(decl, "trying to resolve empty decl path");
	return decl;
}

SemaValue *sema_resolve_decl_path(SemaModule *sema, AstDeclPath *path) {
    SemaScopeDecl *decl = sema_resolve_decl_path_raw(sema, path);
    if (!decl) {
        return NULL;
    }
    switch (decl->type) {
        case SEMA_SCOPE_DECL_MODULE:
            return sema_value_module(decl->module);
        case SEMA_SCOPE_DECL_TYPE:
            return sema_value_type(decl->sema_type);
        case SEMA_SCOPE_DECL_VALUE: {
            SemaValue *result = decl->value_decl.constant ?
                sema_value_const(decl->value_decl.type) :
                sema_value_var(decl->value_decl.type);
            result->integer_expr = decl->value_decl.integer_expr; // TODO: REMOVE THIS!!!
            return result;
        }
    }
	return NULL;
}

SemaValue *sema_resolve_inner_value_path(SemaModule *sema, SemaValue *from, AstInnerPathSegment *segment) {
    segment->sema.value = from;
	switch (segment->type) {
        case AST_INNER_PATH_SEG_SIZEOF: {
            segment->sema.type = SEMA_INNER_PATH_SIZEOF;
            segment->sema.sizeof_type = from->sema_type;
            return sema_value_const(sema_type_primitive_i32());
        }
        case AST_INNER_PATH_SEG_DEREF: {
            if (from->sema_type->type != SEMA_TYPE_POINTER) {
                sema_err("only pointers can be dereferenced, not {sema::type}", from->sema_type);
                return NULL;
            }
            segment->sema.type = SEMA_INNER_PATH_DEREF;
            segment->sema.deref_type = from->sema_type->ptr_to;
            return sema_value_var(from->sema_type->ptr_to);
        }
		case AST_INNER_PATH_SEG_IDENT: {
			if (from->sema_type->type == SEMA_TYPE_STRUCT) {
                segment->sema.type = SEMA_INNER_PATH_STRUCT_MEMBER;
                segment->sema.struct_member.of = from->sema_type;
                SemaStructMember *member = segment->sema.struct_member.member = sema_get_struct_member(sema, from->sema_type->struct_def, &segment->ident);
                if (!member) {
                    return NULL;
                }
                switch (member->type) {
                    case SEMA_STRUCT_MEMBER_FIELD:
                        return sema_value_with_type(from, from->sema_type->struct_def->members[member->field_idx].type->sema);
                    case SEMA_STRUCT_MEMBER_EXT_FUNC:
                        if (from->type != SEMA_VALUE_VAR) {
                            sema_err("ext function can be called on variable only!!!");
                        }
                        return sema_value_ext_func_handle(member->ext_func->type);
                }
                assert(0, "invalid struct member type");
			} else if (from->sema_type->type == SEMA_TYPE_SLICE) {
                Slice length = slice_from_const_cstr("length");
                Slice raw = slice_from_const_cstr("raw");
                segment->sema.slice_type = from->sema_type;
                if (slice_eq(&raw, &segment->ident)) {
                    segment->sema.type = SEMA_INNER_PATH_SLICE_RAW;
                    SemaType *output_type = sema_type_new_pointer(from->sema_type->slice_of);
                    return sema_value_with_type(from, output_type);
                } else if (slice_eq(&length, &segment->ident)) {
                    segment->sema.type = SEMA_INNER_PATH_SLICE_LEN;
                    return sema_value_with_type(from, sema_type_primitive_i32());
                } else {
                    sema_err("{sema::type} has not member {slice}", from->sema_type, &segment->ident);
                    return NULL;
                }
            } else if (from->sema_type->type == SEMA_TYPE_ARRAY) {                Slice length = slice_from_const_cstr("length");
                Slice raw = slice_from_const_cstr("raw");
                if (slice_eq(&length, &segment->ident)) {
                    segment->sema.type = SEMA_INNER_PATH_ARRAY_LEN;
                    segment->sema.array_length = from->sema_type->array.length;
                    return sema_value_const(sema_type_primitive_i32());
                } else {
                    sema_err("{sema::type} has not member {slice}", from->sema_type, &segment->ident);
                    return NULL;
                }
            }
			return NULL;
		}
	}
	return NULL;
}

SemaValue *sema_resolve_inner_type_path(SemaModule *sema, SemaValue *from, AstInnerPathSegment *segment) {
    switch (segment->type) {
        case AST_INNER_PATH_SEG_SIZEOF:
            segment->sema.type = SEMA_INNER_PATH_SIZEOF;
            segment->sema.sizeof_type = from->sema_type;
            return sema_value_const(sema_type_primitive_i32());
        case AST_INNER_PATH_SEG_IDENT:
            sema_err("cannot get a member `{slice}` from type `{sema::type}`", &segment->ident, from->sema_type);
            break;
        case AST_INNER_PATH_SEG_DEREF:
            sema_err("cannot dereference type `{sema::type}`", from->sema_type);
            break;
    }
	return NULL;
}

SemaValue *sema_resolve_inner_path(SemaModule *sema, SemaValue *from, AstInnerPath *path) {
    for (size_t i = 0; i < vec_len(path->segments); i++) {
	    AstInnerPathSegment *segment = &path->segments[i];
        segment->sema.value = from;
        switch (from->type) {
            case SEMA_VALUE_TYPE:
                from = sema_resolve_inner_type_path(sema, from, segment);
                if (!from) {
                    return NULL;
                }
                break;
            case SEMA_VALUE_CONST:
            case SEMA_VALUE_VAR: {
                from = sema_resolve_inner_value_path(sema, from, segment);
                if (!from) {
                    return NULL;
                }
                break;
            }
            case SEMA_VALUE_EXT_FUNC_HANDLE:
        	    sema_err("cannot get path from ext func handle");
                break;
            case SEMA_VALUE_MODULE:
        	    sema_err("cannot get path from module");
                break;
        }
    }
    return from;
}

SemaValue *sema_resolve_path(SemaModule *sema, AstPath *path) {
    SemaValue *value = sema_resolve_decl_path(sema, &path->decl_path);
    if (!value) {
        return NULL;
    }
    return sema_resolve_inner_path(sema, value, &path->inner_path);
}

SemaType *sema_resolve_type_path(SemaModule *sema, AstPath *path) {
    SemaValue *value = sema_resolve_path(sema, path);
    if (!value) {
        return NULL;
    }
    switch (value->type) {
        case SEMA_VALUE_VAR:
        case SEMA_VALUE_CONST:
        case SEMA_VALUE_MODULE:
        case SEMA_VALUE_EXT_FUNC_HANDLE:
            sema_err("{ast::path} is not a type", path);
            return NULL;
        case SEMA_VALUE_TYPE:
            break;
    }
    assert(value->sema_type, "sema type of value was not mapped");
    return value->sema_type;
}
SemaType *sema_resolve_value_path(SemaModule *sema, AstPath *path) {
    SemaValue *value = sema_resolve_path(sema, path);
    if (!value) {
        return NULL;
    }
    switch (value->type) {
        case SEMA_VALUE_VAR:
        case SEMA_VALUE_CONST:
            break;
        case SEMA_VALUE_TYPE:
        case SEMA_VALUE_MODULE:
        case SEMA_VALUE_EXT_FUNC_HANDLE:
            sema_err("{ast::path} is not a value", path);
            return false;
    }
    assert(value->sema_type, "sema type of value was not mapped");
    return value->sema_type;
}
