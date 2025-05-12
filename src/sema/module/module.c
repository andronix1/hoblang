#include "module.h"
#include "core/keymap.h"
#include "core/log.h"
#include "core/mempool.h"
#include "core/null.h"
#include "core/vec.h"
#include "parser/api.h"
#include "parser/parser.h"
#include "lexer/lexer.h"
#include "sema/module/api/decl_handle.h"
#include "sema/module/type.h"
#include "sema/module/value.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static inline bool sema_ss_push(SemaScopeStack *ss, Slice name, SemaDecl decl) {
    assert(vec_len(ss->scopes) > 0);
    return !keymap_insert(vec_top(ss->scopes)->decls_map, name, decl);
}

SemaScopeStack *sema_module_ss_swap(SemaModule *module, SemaScopeStack *ss) {
    SemaScopeStack *old_ss = module->ss;
    module->ss = ss;
    return old_ss;
}

SemaDeclHandle *sema_module_push_decl(SemaModule *module, Slice name, SemaDecl decl) {
    bool succeed;
    if (module->ss) {
        if (decl.is_local) {
            sema_module_err(module, name, "reduntant local specifier in local scope");
        }
        succeed = sema_ss_push(module->ss, name, decl);
    } else {
        succeed = !keymap_insert(module->decls_map, name, decl);
    }
    if (!succeed) {
        sema_module_err(module, name, "`$S` already declared", name);
    }
    return decl.handle;
}

SemaDeclHandle *sema_module_resolve_decl(const SemaModule *module, Slice name) {
    if (module->ss) {
        for (ssize_t i = (ssize_t)vec_len(module->ss->scopes) - 1; i >= 0; i--) {
            SemaDecl *decl = keymap_rev_get(module->ss->scopes[i].decls_map, name);
            if (decl) {
                return decl->handle;
            }
        }
    }
    return NOT_NULL(keymap_rev_get(module->decls_map, name))->handle;
}

void sema_module_push_scope(SemaModule *module) {
    assert(module->ss);
    SemaScope scope = {
        .decls_map = keymap_new_in(module->mempool, SemaDecl)
    };
    vec_push(module->ss->scopes, scope);
}

void sema_module_pop_scope(SemaModule *module) {
    assert(module->ss);
    vec_pop(module->ss->scopes);
}

SemaType *sema_module_returns(const SemaModule *module) {
    assert(module->ss);
    return module->ss->returns;
}

bool sema_module_is_global_scope(const SemaModule *module) {
    return module->ss == NULL;
}

static inline SemaDeclHandle *sema_decl_handle_new(Mempool *mempool, SemaValue *value)
    MEMPOOL_CONSTRUCT(SemaDeclHandle, out->value = value;)

SemaDecl sema_decl_new(SemaModule *module, bool is_local, SemaValue *value) {
    SemaDecl decl = {
        .in = module,
        .is_local = is_local,
        .handle = sema_decl_handle_new(module->mempool, value)
    };
    return decl;
}

void sema_module_push_primitives(SemaModule *module) {
    #define PUSHP(NAME, TYPE) sema_module_push_decl(module, slice_from_cstr(NAME), sema_decl_new(module, true, sema_value_new_type(module->mempool, TYPE)))
    #define PUSHPI(BITS) \
        PUSHP("i" #BITS, sema_type_new_primitive_int(module->mempool, SEMA_PRIMITIVE_INT##BITS, true)); \
        PUSHP("u" #BITS, sema_type_new_primitive_int(module->mempool, SEMA_PRIMITIVE_INT##BITS, false))
    #define PUSHPF(BITS) \
        PUSHP("f" #BITS, sema_type_new_primitive_float(module->mempool, SEMA_PRIMITIVE_FLOAT##BITS));

    PUSHP("bool", sema_type_new_primitive_bool(module->mempool));
    PUSHP("void", sema_type_new_primitive_void(module->mempool));
    PUSHPI(8); PUSHPI(16); PUSHPI(32); PUSHPI(64);
    PUSHPF(32); PUSHPF(64);
}

SemaScopeStack sema_ss_new(SemaModule *module, SemaType *returns) {
    SemaScopeStack ss = {
        .scopes = vec_new_in(module->mempool, SemaScope),
        .returns = returns,
    };
    return ss;
}

void sema_module_err(SemaModule *module, Slice at, const char *fmt, ...) {
    va_list list;
    va_start(list, fmt);
    logs("$s:$L: error: ",
        module->parser->lexer->content->path,
        file_content_locate(module->parser->lexer->content, at).begin);
    logvln(fmt, list);
    logln("$V\n", file_content_get_in_lines_view(module->parser->lexer->content, at));
    va_end(list);
}
