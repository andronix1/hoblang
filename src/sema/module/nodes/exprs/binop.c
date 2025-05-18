#include "binop.h"
#include "ast/api/expr.h"
#include "ast/expr.h"
#include "core/assert.h"
#include "sema/module/api/type.h"
#include "sema/module/module.h"
#include "sema/module/nodes/exprs/binop_ext.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

static SemaBinopKind sema_resolve_binop_kind(AstBinopKindKind kind) {
    switch (kind) {
        case AST_BINOP_ADD: return SEMA_BINOP_ARITHMETIC;
        case AST_BINOP_SUBTRACT: return SEMA_BINOP_ARITHMETIC;
        case AST_BINOP_MULTIPLY: return SEMA_BINOP_ARITHMETIC;
        case AST_BINOP_DIVIDE: return SEMA_BINOP_ARITHMETIC;
        case AST_BINOP_EQUALS: return SEMA_BINOP_EQ;
        case AST_BINOP_NOT_EQUALS: return SEMA_BINOP_EQ;
        case AST_BINOP_LESS: return SEMA_BINOP_COMPARE;
        case AST_BINOP_GREATER: return SEMA_BINOP_COMPARE;
        case AST_BINOP_LESS_EQ: return SEMA_BINOP_COMPARE;
        case AST_BINOP_GREATER_EQ: return SEMA_BINOP_COMPARE;
    }
    UNREACHABLE;
}

SemaValue *sema_module_analyze_binop(SemaModule *module, SemaType *a, SemaType *b, AstBinopKind *binop) {
    SemaBinopKind kind = binop->sema.kind = sema_resolve_binop_kind(binop->kind);
    if (!sema_type_eq(a, b)) {
        sema_module_err(module, binop->slice, "binops can operate equal types, $t and $t were passed", a, b);
        return NULL;
    }
    switch (kind) {
        case SEMA_BINOP_ARITHMETIC: {
            if (sema_type_is_any_int(a)) {
                binop->sema.arithmetic = sema_type_is_uint(a) ?
                    SEMA_BINOP_ARITHMETIC_UINT :
                    SEMA_BINOP_ARITHMETIC_INT;
            } else {
                sema_module_err(module, binop->slice, "cannot use arithmetic binop for $t", a);
            }
            return sema_value_new_final(module->mempool, a);
        }
        case SEMA_BINOP_COMPARE: {
            if (sema_type_is_any_int(a)) {
                binop->sema.compare = sema_type_is_uint(a) ?
                    SEMA_BINOP_COMPARE_UINT :
                    SEMA_BINOP_COMPARE_INT;
            } else {
                sema_module_err(module, binop->slice, "cannot use compare binop for $t", a);
            }
            return sema_value_new_final(module->mempool, sema_type_new_primitive_bool(module->mempool));
        }
        case SEMA_BINOP_EQ:
            if (sema_type_is_any_int(a)) {
                binop->sema.eq = SEMA_BINOP_EQ_INT;
            } else if (sema_type_is_bool(a)) {
                binop->sema.eq = SEMA_BINOP_EQ_BOOL;
            } else {
                sema_module_err(module, binop->slice, "cannot use eq binop for $t", a);
            }
            return sema_value_new_final(module->mempool, sema_type_new_primitive_bool(module->mempool));
    }
    return sema_value_new_final(module->mempool, a);
}
