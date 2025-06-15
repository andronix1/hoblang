#include "dump.h"
#include "core/path.h"
#include "core/vec.h"
#include "ir/dump/code.h"
#include "ir/func.h"
#include "ir/ir.h"
#include <stdio.h>

static void ir_dump_types(IrTypeInfo *types, FILE *stream) {
    for (size_t i = 0; i < vec_len(types); i++) {
        fprintf(stream, "type%lu: ", i);
        IrTypeInfo *info = &types[i];
        switch (info->kind) {
            case IR_TYPE_INFO_SIMPLE: {
                IrType *type = &info->simple;
                switch (type->kind) {
                    case IR_TYPE_VOID: fprintf(stream, "void"); break;
                    case IR_TYPE_BOOL: fprintf(stream, "boolean"); break;
                    case IR_TYPE_FLOAT:
                        switch (type->float_size) {
                            case IR_TYPE_FLOAT_32: fprintf(stream, "32"); break;
                            case IR_TYPE_FLOAT_64: fprintf(stream, "64"); break;
                        }
                        fprintf(stream, "-bit float");
                        break;
                    case IR_TYPE_INT: {
                        switch (type->integer.size) {
                            case IR_TYPE_INT_8: fprintf(stream, "8"); break;
                            case IR_TYPE_INT_16: fprintf(stream, "16"); break;
                            case IR_TYPE_INT_32: fprintf(stream, "32"); break;
                            case IR_TYPE_INT_64: fprintf(stream, "64"); break;
                        }
                        fprintf(stream, "-bit ");
                        fprintf(stream, type->integer.is_signed ? "signed" : "unsigned");
                        fprintf(stream, " integer");
                        break;
                    }
                    case IR_TYPE_FUNCTION:
                        fprintf(stream, "function (");
                        for (size_t i = 0; i < vec_len(type->function.args); i++) {
                            fprintf(stream, i == 0 ? "type%lu" : ", type%lu", type->function.args[i]);
                        }
                        fprintf(stream, ") -> type%lu", type->function.returns);
                        break;
                    case IR_TYPE_POINTER: fprintf(stream, "pointer to type%lu", type->pointer_to); break;
                    case IR_TYPE_STRUCT:
                        fprintf(stream, "structure {");
                        for (size_t i = 0; i < vec_len(type->structure.fields); i++) {
                            fprintf(stream, i == 0 ? " type%lu" : ", type%lu", type->structure.fields[i]);
                        }
                        fprintf(stream, " }");
                        break;
                }
                break;
            }
            case IR_TYPE_INFO_RECORD:
                fprintf(stream, "type%lu", info->record.id);
                break;
        }
        fprintf(stream, "\n");
    }
    fprintf(stream, "\n");
}

void ir_dump_vars(Mempool *mempool, IrVarInfo *vars, FILE *stream) {
    for (size_t i = 0; i < vec_len(vars); i++) {
        IrVarInfo *info = &vars[i];
        fprintf(stream, "decl%lu: var", info->decl_id);
        if (info->var.is_global) {
            fprintf(stream, " `%s`", mempool_slice_to_cstr(mempool, info->var.global_name));
        }
        fprintf(stream, ": type%lu", info->var.type);
        if (info->var.initializer) {
            fprintf(stream, " = ");
            ir_const_dump(info->var.initializer, stream);
        }
        fprintf(stream, "\n");
    }
    fprintf(stream, "\n");
}

void ir_dump_externs(Mempool *mempool, IrExternInfo *externs, FILE *stream) {
    for (size_t i = 0; i < vec_len(externs); i++) {
        IrExternInfo *info = &externs[i];
        fprintf(stream, "decl%lu: external ", info->decl_id);
        switch (info->ext.kind) {
            case IR_EXTERN_FUNC: fprintf(stream, "function"); break;
            case IR_EXTERN_VAR: fprintf(stream, "var"); break;
        }
        fprintf(stream, " `%s`: type%lu\n", mempool_slice_to_cstr(mempool, info->ext.name), info->ext.type);
    }
    fprintf(stream, "\n");
}

void ir_dump_functions(Mempool *mempool, IrFuncInfo *funcs, FILE *stream) {
    for (size_t i = 0; i < vec_len(funcs); i++) {
        IrFuncInfo *info = &funcs[i];
        fprintf(stream, "decl%lu: ", info->decl_id);
        if (info->func.is_global) {
            fprintf(stream, "global function `%s`", mempool_slice_to_cstr(mempool, info->func.global_name));
        } else {
            fprintf(stream, "function");
        }
        fprintf(stream, "(");
        for (size_t i = 0; i < vec_len(info->args); i++) {
            if (i != 0) fprintf(stream, ", ");
            fprintf(stream, "local%lu", info->args[i]);
        }
        fprintf(stream, "): type%lu [", info->func.type_id);
        if (vec_len(info->locals)) {
            fprintf(stream, "\n");
            for (size_t i = 0; i < vec_len(info->locals); i++) {
                IrFuncLocal *local = &info->locals[i];
                fprintf(stream, "  local%lu(%s): type%lu\n", i, local->mutability == IR_MUTABLE ? "mut" : "imm",
                    local->type);
            }
        }
        fprintf(stream, "] ");
        ir_code_dump(info->func.code, stream);
        fprintf(stream, "\n");
    }
}

bool ir_dump(Ir *ir, const Path output) {
    FILE *stream = fopen(output, "w");
    if (!stream) {
        return false;
    }
    ir_dump_types(ir->types, stream);
    ir_dump_vars(ir->mempool, ir->vars, stream);
    ir_dump_externs(ir->mempool, ir->externs, stream);
    ir_dump_functions(ir->mempool, ir->funcs, stream);
    fclose(stream);
    return true;
}
