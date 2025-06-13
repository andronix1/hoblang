#include "module.h"
#include "core/file_content.h"
#include "core/keymap.h"
#include "core/log.h"
#include "core/path.h"
#include "ir/api/ir.h"
#include "parser/parser.h"
#include "lexer/lexer.h"
#include "sema/module/api/decl.h"
#include "sema/module/api/type.h"
#include "sema/module/scope.h"
#include <stdarg.h>
#include <stdio.h>

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

Path sema_module_file_path(SemaModule *module) {
    return sema_module_file_content(module)->path;
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

void sema_module_link_project(SemaModule *module, SemaProject *project) {
    module->project = project;
}

SemaDecl *sema_module_resolve_req_decl(SemaModule *module, Slice name) {
    if (module->ss) {
        SemaDecl *decl = sema_ss_resolve_decl(module->ss, name);
        if (decl) {
            return decl;
        }
    }
    SemaDecl **decl = keymap_get(module->local_decls_map, name);
    if (!decl) {
        sema_module_err(module, name, "cannot find declaration `$S`", name);
        return NULL;
    }
    return *decl;
}

void sema_module_push_loop(SemaModule *module, SemaLoop loop) {
    assert(module->ss);
    if (!sema_ss_try_push_loop(module->ss, loop)) {
        assert(loop.is_labeled);
        sema_module_err(module, loop.label, "labeled loop `$S` already exists", loop.label);
    }
}

void sema_module_pop_loop(SemaModule *module) {
    assert(module->ss);
    sema_ss_pop_loop(module->ss);
}

void sema_module_push_scope(SemaModule *module) {
    assert(module->ss);
    sema_ss_push_scope(module->ss, module->mempool);
}

void sema_module_pop_scope(SemaModule *module) {
    assert(module->ss);
    sema_ss_pop_scope(module->ss);
}

SemaScopeStack *sema_module_swap_ss(SemaModule *module, SemaScopeStack *new_ss) {
    SemaScopeStack *ss = module->ss;
    module->ss = new_ss;
    return ss;
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

inline bool sema_module_is_global_scope(SemaModule *module) {
    return module->ss == NULL;
}

void sema_module_push_decl(SemaModule *module, Slice name, SemaDecl *decl) {
    if (sema_module_is_global_scope(module)) {
        if (keymap_insert(module->local_decls_map, name, decl)) {
            sema_module_err(module, name, "`$S` already declared", name);
        }
    } else {
        sema_ss_push_decl(module, module->ss, name, decl);
    }
}

IrTypeId sema_module_get_type_id(SemaModule *module, SemaTypeId id) {
    return module->types[id].id;
}
