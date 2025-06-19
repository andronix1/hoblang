#include "module.h"
#include "core/assert.h"
#include "core/file_content.h"
#include "core/keymap.h"
#include "core/log.h"
#include "core/path.h"
#include "parser/parser.h"
#include "lexer/lexer.h"
#include "sema/module/api/decl.h"
#include "sema/module/api/type.h"
#include "sema/module/scope.h"
#include "sema/module/decl.h"
#include <stdarg.h>
#include <stdio.h>

static inline SemaTypeInfo sema_type_info_new(HirTypeId id) {
    SemaTypeInfo info = {
        .id = id,
        .type = NULL
    };
    return info;
}

static inline const FileContent *sema_module_file_content(SemaModule *module) {
    return module->parser->lexer->content;
}

Slice sema_module_internal_slice() {
    return slice_new(NULL, 0);
}

Path sema_module_file_path(SemaModule *module) {
    return sema_module_file_content(module)->path;
}

void sema_module_err(SemaModule *module, Slice where, const char *fmt, ...) {
    module->failed = true;
    va_list list;
    va_start(list, fmt);
    if (where.value == NULL) {
        logs("error: ");
        logv(fmt, list);
    } else {
        const FileContent *content = sema_module_file_content(module);
        logs("$L: error: ", file_content_locate(content, where).begin);
        logv(fmt, list);
        logln("\n$V", file_content_get_in_lines_view(content, where));
    }
    logs("\n");
    va_end(list);
}

void sema_module_link_project(SemaModule *module, SemaProject *project) {
    module->project = project;
}

void sema_module_make_no_std(SemaModule *module) {
    module->no_std = true;
}

SemaDecl *sema_module_resolve_req_decl(SemaModule *module, Slice name) {
    return sema_module_resolve_req_decl_from(module, module, name);
}

SemaDecl *sema_module_resolve_req_decl_from_at(SemaModule *module, SemaModule *from, Slice at, Slice name) {
    if (module->ss) {
        SemaDecl *decl = sema_ss_resolve_decl(module->ss, name);
        if (decl) {
            return decl;
        }
    }
    SemaDecl **ptr = keymap_get(module->local_decls_map, name);
    if (!ptr) {
        sema_module_err(from, at, "cannot find declaration `$S`", name);
        return NULL;
    }
    SemaDecl *decl = *ptr;
    if (decl->module && decl->module != from) {
        sema_module_err(from, at, "`$S` is private", name);
        return NULL;
    }
    return decl;
}

SemaDecl *sema_module_resolve_req_decl_from(SemaModule *module, SemaModule *from, Slice name) {
    return sema_module_resolve_req_decl_from_at(module, from, name, name);
}

void sema_module_push_scope(SemaModule *module, SemaLoop *loop) {
    assert(module->ss);
    if (loop && loop->is_labeled && sema_ss_labeled_loop(module->ss, loop->label)) {
        sema_module_err(module, loop->label, "labeled loop `$S` already exists", loop->label);
        loop = NULL;
    }
    sema_ss_push_scope(module->ss, loop, module->mempool);
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
    vec_push(module->types, sema_type_info_new(hir_register_type(module->hir)));
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
        if (!decl->module) {
            sema_module_err(module, name, "trying to specify visibility in local scope");
        }
        sema_ss_push_decl(module, module->ss, name, decl);
    }
}

static inline void sema_module_emit_defers_before_opt_loop(SemaModule *module, HirLoopId *id) {
    for (ssize_t i = (ssize_t)vec_len(module->ss->scopes) - 1; i >= 0; i--) {
        SemaScope *scope = &module->ss->scopes[i];
        for (ssize_t j = (ssize_t)vec_len(scope->defers) - 1; j >= 0; j--) {
            sema_ss_append_body(module->ss, scope->defers[j]);
        }
        if (id && scope->loop && scope->loop->id == *id) {
            return;
        }
    }
    if (id) UNREACHABLE;
}

bool sema_module_scope_breaks(SemaModule *module) {
    SemaScope *scope = vec_top(module->ss->scopes);
    return scope->breaks;
}

void sema_module_scope_break(SemaModule *module) {
    SemaScope *scope = vec_top(module->ss->scopes);
    scope->breaks = true;
}

void sema_module_emit_defers_before_loop(SemaModule *module, HirLoopId id) {
    return sema_module_emit_defers_before_opt_loop(module, &id);
}

void sema_module_emit_current_defers(SemaModule *module) {
    assert(module->ss && vec_len(module->ss->scopes));
    SemaScope *scope = vec_top(module->ss->scopes);
    for (ssize_t i = (ssize_t)vec_len(scope->defers) - 1; i >= 0; i--) {
        sema_ss_append_body(module->ss, scope->defers[i]);
    }
}

void sema_module_emit_defers(SemaModule *module) {
    return sema_module_emit_defers_before_opt_loop(module, NULL);
}

void sema_module_add_defer(SemaModule *module, HirCode *code) {
    assert(module->ss);
    sema_ss_push_defer(module->ss, code);
}

HirTypeId sema_module_get_type_id(SemaModule *module, SemaTypeId id) {
    return module->types[id].id;
}
