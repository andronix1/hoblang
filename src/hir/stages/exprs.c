#include "exprs.h"
#include "core/assert.h"
#include "hir/api/code.h"
#include "hir/api/expr.h"
#include "hir/api/hir.h"
#include "hir/api/type.h"
#include "hir/hir.h"
#include <alloca.h>
#include <assert.h>
#include <stdio.h>

HirTypeId hir_resolve_decl_type_id(Hir *hir, HirDeclId id) {
    HirDeclInfo *info = &hir->decls[id];
    assert(info->filled);
    switch (info->kind) {
        case HIR_DECL_FUNC:
            return hir_get_func_info(hir, info->func)->type;
        case HIR_DECL_EXTERN: {
            keymap_at(hir->externs_map, info->external, ext);
            return ext->value.type;
        }
        case HIR_DECL_VAR:
            return hir_get_var_info(hir, info->func)->type;
    }
    UNREACHABLE;
}

static inline HirTypeId hir_get_binop_type(Hir *hir, HirExprStep *steps, HirBinop *binop) {
    switch (binop->kind) {
        case HIR_BINOP_ARITHMETIC:
            return steps[binop->ls].type;
        case HIR_BINOP_ORDER:
        case HIR_BINOP_COMPARE:
        case HIR_BINOP_BOOL:
            return hir_add_type(hir, hir_type_new_bool());
        case HIR_BINOP_INT:
            return steps[binop->ls].type;
    }
    UNREACHABLE;
}

static HirTypeId hir_get_expr_step_type(Hir *hir, HirFuncId func, HirExprStep *steps, size_t idx) {
    HirExprStep *step = &steps[idx];
    switch (step->kind) {
        case HIR_EXPR_STEP_GET_DECL: return hir_resolve_decl_type_id(hir, step->decl_id);
        case HIR_EXPR_STEP_CALL: {
            HirType *type = hir_resolve_simple_type(hir, steps[step->call.callable].type);
            assert(type->kind == HIR_TYPE_FUNCTION);
            return type->function.returns;
        }
        case HIR_EXPR_STEP_CONST: return step->constant.type;
        case HIR_EXPR_STEP_BINOP: return hir_get_binop_type(hir, steps, &step->binop);
        case HIR_EXPR_STEP_TAKE_REF: return hir_add_type(hir, hir_type_new_pointer(steps[step->ref_step].type));
        case HIR_EXPR_STEP_DEREF: {
            HirType *type = hir_resolve_simple_type(hir, steps[step->deref_step].type);
            assert(type->kind == HIR_TYPE_POINTER);
            return type->pointer_to;
        }
        case HIR_EXPR_STEP_GET_LOCAL: {
            const HirFuncInfo *info = hir_get_func_info(hir, func);
            return info->locals[step->local_id].type;
        }
        case HIR_EXPR_STEP_CAST_INT: return step->cast_int.dest;
        case HIR_EXPR_STEP_BUILD_STRUCT: return step->build_struct.type;
        case HIR_EXPR_STEP_STRUCT_FIELD: {
            HirType *type = hir_resolve_simple_type(hir, steps[step->struct_field.step].type);
            assert(type->kind == HIR_TYPE_STRUCT);
            return type->structure.fields[step->struct_field.idx].type;
        }
        case HIR_EXPR_STEP_STRING:
            return hir_add_type(hir, hir_type_new_pointer(hir_add_type(hir, hir_type_new_int(HIR_TYPE_INT_8, false))));
        case HIR_EXPR_STEP_BOOL_SKIP:
        case HIR_EXPR_STEP_NOT:
            return hir_add_type(hir, hir_type_new_bool());
        case HIR_EXPR_STEP_SIZEOF: return step->size.type;
        case HIR_EXPR_STEP_BUILD_ARRAY:
            return hir_add_type(hir, hir_type_new_array(step->build_array.type, vec_len(step->build_array.elements)));
        case HIR_EXPR_STEP_IDX_ARRAY: {
            HirType *type = hir_resolve_simple_type(hir, steps[step->idx_array.value].type);
            assert(type->kind == HIR_TYPE_ARRAY);
            return type->array.of;
        }
        case HIR_EXPR_STEP_IDX_POINTER: {
            HirType *type = hir_resolve_simple_type(hir, steps[step->idx_pointer.value].type);
            assert(type->kind == HIR_TYPE_POINTER);
            return type->pointer_to;
        }
        case HIR_EXPR_STEP_NEG: return steps[step->neg.step].type;
        case HIR_EXPR_STEP_GEN_FUNC:
            step->gen_func.usage = hir_add_gen_scope_usage(hir, step->gen_func.scope, step->gen_func.params);
            return step->gen_func.type;
    }
    UNREACHABLE;
}

static void hir_fill_expr(Hir *hir, HirFuncId func, HirExpr *expr) {
    for (size_t i = 0; i < vec_len(expr->steps); i++) {
        expr->steps[i].type = hir_get_expr_step_type(hir, func, expr->steps, i);    
    }
}

static void hir_fill_code(Hir *hir, HirFuncId func, HirCode *code);

static inline bool _ir_fill_stmt_cond_jmp(Hir *hir, HirFuncId func, HirStmtCondJmp *cond_jmp) {
    bool passed = cond_jmp->else_code == NULL;
    size_t count = vec_len(cond_jmp->conds) + 1;
    bool *flows = passed ? NULL : alloca(sizeof(bool) * count);
    for (size_t i = 0; i < vec_len(cond_jmp->conds); i++) {
        HirStmtCondJmpBlock *block = &cond_jmp->conds[i];
        hir_fill_expr(hir, func, &block->cond);
        hir_fill_code(hir, func, block->code);
        if (!passed) {
            flows[i] = block->code->breaks;
        }
    }
    if (cond_jmp->else_code) {
        hir_fill_code(hir, func, cond_jmp->else_code);
        flows[count - 1] = cond_jmp->else_code->breaks;
    }
    if (passed) {
        return false;
    }
    for (size_t i = 0; i < count; i++) {
        if (!flows[i]) return false;
    }
    return true;
}

static bool hir_fill_stmt_cond_jmp(Hir *hir, HirFuncId func, HirStmtCondJmp *cond_jmp) {
    return cond_jmp->breaks = _ir_fill_stmt_cond_jmp(hir, func, cond_jmp);
}


static bool hir_fill_stmt(Hir *hir, HirFuncId func, HirStmt *stmt) {
    switch (stmt->kind) {
        case HIR_STMT_EXPR:
            hir_fill_expr(hir, func, &stmt->expr);
            return false;
        case HIR_STMT_RET:
            hir_fill_expr(hir, func, &stmt->ret.value);
            return true;
        case HIR_STMT_STORE:
            hir_fill_expr(hir, func, &stmt->store.lvalue);
            hir_fill_expr(hir, func, &stmt->store.rvalue);
            return false;
        case HIR_STMT_INIT_FINAL:
            hir_fill_expr(hir, func, &stmt->init_final.value);
            return false;
        case HIR_STMT_DECL_VAR:
            return false;
        case HIR_STMT_RET_VOID:
            return true;
        case HIR_STMT_COND_JMP:
            return hir_fill_stmt_cond_jmp(hir, func, &stmt->cond_jmp);
        case HIR_STMT_LOOP:
            hir_fill_code(hir, func, stmt->loop.code);
            // TODO
            return false;
        case HIR_STMT_BREAK:
            return true;
        case HIR_STMT_CONTINUE:
            return true;
    }
    UNREACHABLE;
}

static void hir_fill_code(Hir *hir, HirFuncId func, HirCode *code) {
    for (size_t i = 0; i < vec_len(code->stmts); i++) {
        bool breaks = hir_fill_stmt(hir, func, &code->stmts[i]);    
        if (breaks) {
            assert(i == vec_len(code->stmts) - 1);
            code->breaks = breaks;
        }
    }
}

void hir_fill_exprs(Hir *hir) {
    for (size_t i = 0; i < vec_len(hir->funcs); i++) {
        HirFuncRecord *info = &hir->funcs[i];
        assert(info->filled);
        hir_fill_code(hir, i, info->info.code);
    }
}
