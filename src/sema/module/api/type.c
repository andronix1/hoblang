#include "type.h"
#include "core/assert.h"
#include "core/keymap.h"
#include "core/log.h"
#include "core/slice.h"
#include "sema/module/type.h"
#include <stdio.h>

bool sema_type_eq(const SemaType *a, const SemaType *b) {
    if (a->kind != b->kind) {
        return false;
    }
    switch (a->kind) {
        case SEMA_TYPE_PRIMITIVE:
            if (a->primitive.kind != b->primitive.kind) {
                return false;
            }
            switch (a->primitive.kind) {
                case SEMA_PRIMITIVE_INT:
                    return
                        a->primitive.integer.size == b->primitive.integer.size &&
                        a->primitive.integer.is_signed == b->primitive.integer.is_signed;
                case SEMA_PRIMITIVE_FLOAT:
                    return a->primitive.fp == b->primitive.fp;
                case SEMA_PRIMITIVE_VOID:
                case SEMA_PRIMITIVE_BOOL:
                    return true;
            }
        case SEMA_TYPE_FUNCTION:
            if (vec_len(a->function.args) != vec_len(b->function.args)) {
                return false;
            }
            for (size_t i = 0; i < vec_len(b->function.args); i++) {
                if (!sema_type_eq(a->function.args[i], b->function.args[i])) {
                    return false;
                }
            }
            return sema_type_eq(a->function.returns, b->function.returns);
        case SEMA_TYPE_STRUCT:
            if (vec_len(a->structure.fields_map) != vec_len(b->structure.fields_map)) {
                return false;
            }
            for (size_t i = 0; i < vec_len(a->structure.fields_map); i++) {
                keymap_at(a->structure.fields_map, i, af);
                keymap_at(b->structure.fields_map, i, bf);
                if (
                    !slice_eq(af->key, bf->key) ||
                    af->value.is_local != bf->value.is_local ||
                    !sema_type_eq(af->value.type, bf->value.type)) {
                    return false;
                }
            }
            return true;
        case SEMA_TYPE_POINTER: return sema_type_eq(a->pointer_to, b->pointer_to);
    }
    UNREACHABLE;
}

void sema_type_print(va_list list) {
    SemaType *type = va_arg(list, SemaType*);
    switch (type->kind) {
        case SEMA_TYPE_PRIMITIVE:
            switch (type->primitive.kind) {
                case SEMA_PRIMITIVE_INT: {
                    const char *s = type->primitive.integer.is_signed ? "i" : "u";
                    switch (type->primitive.integer.size) {
                        case SEMA_PRIMITIVE_INT8: printf("%s8", s); return;
                        case SEMA_PRIMITIVE_INT16: printf("%s16", s); return;
                        case SEMA_PRIMITIVE_INT32: printf("%s32", s); return;
                        case SEMA_PRIMITIVE_INT64: printf("%s64", s); return;
                    }
                    UNREACHABLE;
                }
                case SEMA_PRIMITIVE_FLOAT:
                    switch (type->primitive.fp) {
                        case SEMA_PRIMITIVE_FLOAT32: printf("f32"); return;
                        case SEMA_PRIMITIVE_FLOAT64: printf("f64"); return;
                    }
                    UNREACHABLE;
                case SEMA_PRIMITIVE_VOID: printf("void"); return;
                case SEMA_PRIMITIVE_BOOL: printf("bool"); return;
            }
            UNREACHABLE;
        case SEMA_TYPE_FUNCTION:
            printf("fun (");
            for (size_t i = 0; i < vec_len(type->function.args); i++) {
                if (i != 0) {
                    printf(", ");
                }
                logs("$t", type->function.args[i]);
            }
            logs(") -> $t", type->function.returns);
            return;
        case SEMA_TYPE_STRUCT:
            TODO;
        case SEMA_TYPE_POINTER:
            logs("*$t", type->pointer_to);
            return;
    }
    UNREACHABLE;
}
