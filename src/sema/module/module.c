#include "module.h"
#include "core/attributes.h"
#include "core/file_content.h"
#include "core/keymap.h"
#include "core/log.h"
#include "ir/api/ir.h"
#include "parser/parser.h"
#include "lexer/lexer.h"
#include "sema/module/api/decl.h"
#include "sema/module/api/type.h"
#include <stdarg.h>

static inline SemaTypeInfo sema_type_info_new(IrTypeId id) {
    SemaTypeInfo info = {
        .id = id,
        .type = NULL
    };
    return info;
}

static inline const FileContent *sema_module_file_content(SemaModule *module) {
    return module->parser->lexer->content;
}

void sema_module_err(SemaModule *module, Slice where, const char *fmt, ...) {
    module->failed = true;
    va_list list;
    va_start(list, fmt);
    const FileContent *content = sema_module_file_content(module);
    logs("$L: error: ", file_content_locate(content, where).begin);
    logv(fmt, list);
    logln("\n$V", file_content_get_in_lines_view(content, where));
    va_end(list);
}

SemaDecl *sema_module_resolve_req_decl(SemaModule *module, Slice name) {
    SemaDecl **decl = keymap_get(module->local_decls_map, name);
    if (!decl) {
        sema_module_err(module, name, "cannot find declaration `$S`", name);
        return NULL;
    }
    return *decl;
}

SemaTypeId sema_module_register_type_alias(SemaModule *module) {
    vec_push(module->types, sema_type_info_new(ir_add_type_record(module->ir)));
    return vec_len(module->types) - 1;
}

void sema_module_init_type_alias(SemaModule *module, SemaTypeId id, SemaType *type) {
    SemaTypeInfo *info = &module->types[id];
    assert(!info->type);
    info->type = type;
}

void sema_module_push_decl(SemaModule *module, Slice name, SemaDecl *decl) {
    if (keymap_insert(module->local_decls_map, name, decl)) {
        sema_module_err(module, name, "`$S` already declared", name);
    }
}

IrTypeId sema_module_get_type_id(SemaModule *module, SemaTypeId id) {
    return module->types[id].id;
}

bool sema_module_is_global_scope(SemaModule *module UNUSED) {
    // TODO
    return true;
}
