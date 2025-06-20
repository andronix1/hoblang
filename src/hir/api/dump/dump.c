#include "dump.h"
#include "core/path.h"
#include "core/vec.h"
#include "hir/api/dump/code.h"
#include "hir/hir.h"
#include <stdio.h>

static void hir_dump_types(HirTypeInfo *types, FILE *stream) {
    for (size_t i = 0; i < vec_len(types); i++) {
        fprintf(stream, "type%lu: ", i);
        HirTypeInfo *info = &types[i];
        switch (info->kind) {
            case HIR_TYPE_INFO_SIMPLE: {
                HirType *type = &info->simple;
                switch (type->kind) {
                    case HIR_TYPE_VOID: fprintf(stream, "void"); break;
                    case HIR_TYPE_BOOL: fprintf(stream, "boolean"); break;
                    case HIR_TYPE_FLOAT:
                        switch (type->float_size) {
                            case HIR_TYPE_FLOAT_32: fprintf(stream, "32"); break;
                            case HIR_TYPE_FLOAT_64: fprintf(stream, "64"); break;
                        }
                        fprintf(stream, "-bit float");
                        break;
                    case HIR_TYPE_INT: {
                        switch (type->integer.size) {
                            case HIR_TYPE_INT_8: fprintf(stream, "8"); break;
                            case HIR_TYPE_INT_16: fprintf(stream, "16"); break;
                            case HIR_TYPE_INT_32: fprintf(stream, "32"); break;
                            case HIR_TYPE_INT_64: fprintf(stream, "64"); break;
                        }
                        fprintf(stream, "-bit ");
                        fprintf(stream, type->integer.is_signed ? "signed" : "unsigned");
                        fprintf(stream, " integer");
                        break;
                    }
                    case HIR_TYPE_FUNCTION:
                        fprintf(stream, "function (");
                        for (size_t i = 0; i < vec_len(type->function.args); i++) {
                            fprintf(stream, i == 0 ? "type%lu" : ", type%lu", type->function.args[i]);
                        }
                        fprintf(stream, ") -> type%lu", type->function.returns);
                        break;
                    case HIR_TYPE_POINTER: fprintf(stream, "pointer to type%lu", type->pointer_to); break;
                    case HIR_TYPE_ARRAY: fprintf(stream, "type%lu[%lu]", type->array.of, type->array.length); break;
                    case HIR_TYPE_STRUCT:
                        fprintf(stream, "structure {");
                        for (size_t i = 0; i < vec_len(type->structure.fields); i++) {
                            fprintf(stream, i == 0 ? " type%lu" : ", type%lu", type->structure.fields[i].type);
                        }
                        fprintf(stream, " }");
                        break;
                    case HIR_TYPE_GEN_PARAM: fprintf(stream, "genParam%lu", type->gen_param_id); break;
                    case HIR_TYPE_GEN:
                        fprintf(stream, "genType%lu<", type->gen.id);
                        for (size_t i = 0; i < vec_len(type->gen.params); i++) {
                            fprintf(stream, i == 0 ? " type%lu" : ", type%lu", type->gen.params[i]);
                        }
                        fprintf(stream, ">");
                        break;
                }
                break;
            }
            case HIR_TYPE_INFO_RECORD:
                fprintf(stream, "type%lu", info->record.id);
                break;
        }
        fprintf(stream, "\n");
    }
    fprintf(stream, "\n");
}

void hir_dump_vars(Hir *hir, FILE *stream) {
    for (size_t i = 0; i < vec_len(hir->vars); i++) {
        HirVarInfo *info = &hir->vars[i];
        fprintf(stream, "var%lu", i);
        if (info->global_name.has_value) {
            fprintf(stream, " `%s`", mempool_slice_to_cstr(hir->mempool, info->global_name.slice));
        }
        fprintf(stream, ": type%lu", info->type);
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
        fprintf(stream, " `%s`: type%lu\n", mempool_slice_to_cstr(hir->mempool, rec->key), rec->value.type);
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
        const HirType *type = hir_resolve_simple_type(hir, info->type);
        assert(type->kind == HIR_TYPE_FUNCTION);
        fprintf(stream, "(");
        for (size_t i = 0; i < vec_len(type->function.args); i++) {
            if (i != 0) fprintf(stream, ", ");
            fprintf(stream, "local%lu", type->function.args[i]);
        }
        fprintf(stream, "): type%lu [", info->type);
        if (vec_len(info->locals)) {
            fprintf(stream, "\n");
            for (size_t i = 0; i < vec_len(info->locals); i++) {
                HirFuncLocal *local = &info->locals[i];
                fprintf(stream, "  local%lu(%s): type%lu\n", i, local->mutability == HIR_MUTABLE ? "mut" : "imm",
                    local->type);
            }
        }
        fprintf(stream, "] ");
        hir_code_dump(info->code, stream);
        fprintf(stream, "\n");
    }
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
    hir_dump_types(hir->types, stream);
    hir_dump_decls(hir, stream);
    hir_dump_vars(hir, stream);
    hir_dump_externs(hir, stream);
    hir_dump_functions(hir, stream);
    fclose(stream);
    return true;
}
