#include "dump.h"
#include "core/path.h"
#include "core/vec.h"
#include "hir/api/dump/code.h"
#include "hir/api/dump/type.h"
#include "hir/hir.h"
#include <stdio.h>

void hir_dump_vars(Hir *hir, FILE *stream) {
    for (size_t i = 0; i < vec_len(hir->vars); i++) {
        HirVarInfo *info = &hir->vars[i];
        fprintf(stream, "var%lu", i);
        if (info->global_name.has_value) {
            fprintf(stream, " `%s`", mempool_slice_to_cstr(hir->mempool, info->global_name.slice));
        }
        fprintf(stream, ": ");
        hir_dump_type(info->type, stream);
        if (info->initialized) {
            fprintf(stream, " = ");
            hir_const_dump(&info->initializer, stream);
        }
        fprintf(stream, "\n");
    }
    fprintf(stream, "\n");
}

void hir_dump_externs(Hir *hir, FILE *stream) {
    for (size_t i = 0; i < vec_len(hir->externs_map); i++) {
        keymap_at(hir->externs_map, i, rec);
        fprintf(stream, "external%lu:", i);
        switch (rec->value.kind) {
            case HIR_EXTERN_FUNC: fprintf(stream, "function"); break;
            case HIR_EXTERN_VAR: fprintf(stream, "var"); break;
        }
        fprintf(stream, " `%s`: ", mempool_slice_to_cstr(hir->mempool, rec->key));
        hir_dump_type(rec->value.type, stream);
        fprintf(stream, "\n");
    }
    fprintf(stream, "\n");
}

void hir_dump_functions(Hir *hir, FILE *stream) {
    for (size_t i = 0; i < vec_len(hir->funcs); i++) {
        const HirFuncInfo *info = hir_get_func_info(hir, i);
        fprintf(stream, "func%lu: ", i);
        if (info->global_name.has_value) {
            fprintf(stream, "`%s` ", mempool_slice_to_cstr(hir->mempool, info->global_name.slice));
        }
        assert(info->type->kind == HIR_TYPE_FUNCTION);
        hir_dump_type(info->type, stream);
        fprintf(stream, " [");
        if (vec_len(info->locals)) {
            fprintf(stream, "\n");
            for (size_t i = 0; i < vec_len(info->locals); i++) {
                HirFuncLocal *local = &info->locals[i];
                fprintf(stream, "  local%lu(%s): ", i, local->mutability == HIR_MUTABLE ? "mut" : "imm");
                hir_dump_type(local->type, stream);
                fprintf(stream, "\n");
            }
        }
        fprintf(stream, "] ");
        hir_code_dump(info->code, stream);
        fprintf(stream, "\n");
    }
}

static void hir_dump_gens(Hir *hir, FILE *stream) {
    fprintf(stream, "rootGenScope -> (");
    for (size_t i = 0; i < vec_len(hir->root_gen_scopes); i++) {
        if (i != 0) fprintf(stream, ", ");
        fprintf(stream, "genScope%lu", hir->root_gen_scopes[i]);
    }
    fprintf(stream, ")\n");

    for (size_t i = 0; i < vec_len(hir->gen_scopes); i++) {
        HirGenScope *scope = &hir->gen_scopes[i];
        fprintf(stream, "genScope%lu<", i);
        for (size_t i = 0; i < vec_len(scope->params); i++) {
            if (i != 0) fprintf(stream, ", ");
            fprintf(stream, "genParam%lu", scope->params[i]);
        }
        fprintf(stream, "> -> (");
        for (size_t i = 0; i < vec_len(scope->scopes); i++) {
            if (i != 0) fprintf(stream, ", ");
            fprintf(stream, "genScope%lu", scope->scopes[i]);
        }
        fprintf(stream, "): ");
        for (size_t i = 0; i < vec_len(scope->funcs); i++) {
            if (i != 0) fprintf(stream, ", ");
            fprintf(stream, "func%lu", scope->funcs[i]);
        }
        fprintf(stream, "\n");
    }
    fprintf(stream, "\n");
}

static void hir_dump_decls(Hir *hir, FILE *stream) {
    for (size_t i = 0; i < vec_len(hir->decls); i++) {
        HirDeclInfo *info = &hir->decls[i];
        fprintf(stream, "decl%lu: ", i);
        switch (info->kind) {
            case HIR_DECL_FUNC: fprintf(stream, "func%lu", info->func); break;
            case HIR_DECL_EXTERN: fprintf(stream, "extern%lu", info->external); break;
            case HIR_DECL_VAR: fprintf(stream, "var%lu", info->var); break;
        }
        fprintf(stream, "\n");
    }
    fprintf(stream, "\n");
}

bool hir_dump(Hir *hir, const Path output) {
    FILE *stream = fopen(output, "w");
    if (!stream) {
        return false;
    }
    hir_dump_decls(hir, stream);
    hir_dump_vars(hir, stream);
    hir_dump_externs(hir, stream);
    hir_dump_functions(hir, stream);
    hir_dump_gens(hir, stream);
    fclose(stream);
    return true;
}
