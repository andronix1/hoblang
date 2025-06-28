#include "type.h"

void hir_dump_type(HirType *type, FILE *stream) {
    switch (type->kind) {
        case HIR_TYPE_VOID: fprintf(stream, "void"); break;
        case HIR_TYPE_BOOL: fprintf(stream, "bool"); break;
        case HIR_TYPE_INT:
            fprintf(stream, type->integer.is_signed ? "i" : "u");
            switch (type->integer.size) {
                case HIR_TYPE_INT_8: fprintf(stream, "8"); break;
                case HIR_TYPE_INT_16: fprintf(stream, "16"); break;
                case HIR_TYPE_INT_32: fprintf(stream, "32"); break;
                case HIR_TYPE_INT_64: fprintf(stream, "64"); break;
            }
            break;
        case HIR_TYPE_FLOAT:
            fprintf(stream, "f");
            switch (type->float_size) {
                case HIR_TYPE_FLOAT_32: fprintf(stream, "32"); break;
                case HIR_TYPE_FLOAT_64: fprintf(stream, "64"); break;
            }
            break;
        case HIR_TYPE_FUNCTION:
            fprintf(stream, "fun (");
            for (size_t i = 0; i < vec_len(type->function.args); i++) {
                if (i != 0) fprintf(stream, ", ");
                hir_dump_type(type->function.args[i], stream);
            }
            fprintf(stream, ") -> ");
            hir_dump_type(type->function.returns, stream);
            break;
        case HIR_TYPE_POINTER:
            fprintf(stream, "*");
            hir_dump_type(type->pointer_to, stream);
            break;
        case HIR_TYPE_ARRAY:
            fprintf(stream, "[%lu]", type->array.length);
            hir_dump_type(type->array.of, stream);
            break;
        case HIR_TYPE_STRUCT:
            fprintf(stream, "{");
            for (size_t i = 0; i < vec_len(type->function.args); i++) {
                fprintf(stream, i == 0 ? " " : ", ");
                hir_dump_type(type->structure.fields[i].type, stream);
            }
            fprintf(stream, " }");
            break;
        case HIR_TYPE_GEN:
            fprintf(stream, "<%lu>", type->gen_param);
            break;
    }
}
