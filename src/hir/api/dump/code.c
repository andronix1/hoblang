#include "code.h"
#include "hir/api/dump/type.h"
#include <stdio.h>

static inline void ftabs(FILE *stream, size_t tabs) {
    for (size_t i = 0; i < tabs; i++) fprintf(stream, "  ");
}

void hir_const_dump(HirConst *constant, FILE *stream) {
    hir_dump_type(constant->type, stream);
    fprintf(stream, " ");
    switch (constant->kind) {
        case HIR_CONST_BOOL: fprintf(stream, constant->boolean ? "true" : "false"); break;
        case HIR_CONST_INT: fprintf(stream, "%lu", constant->integer); break;
        case HIR_CONST_REAL: fprintf(stream, "%Lf", constant->real); break;
        case HIR_CONST_STRUCT:
            fprintf(stream, "{");
            for (size_t i = 0; i < vec_len(constant->struct_fields); i++) {
                fprintf(stream, i == 0 ? " " : ", ");
                hir_const_dump(&constant->struct_fields[i], stream);
            }
            fprintf(stream, " }");
            break;
        case HIR_CONST_FUNC: fprintf(stream, "func decl%lu", constant->func_decl); break;
        case HIR_CONST_UNDEFINED: fprintf(stream, "undefined"); break;
        case HIR_CONST_STRING_PTR: fprintf(stream, "<string>"); break;
        case HIR_CONST_GEN_FUNC:
            fprintf(stream, "genScope%lu.genFunc%lu.<", constant->gen_func.scope, constant->gen_func.func);
            for (size_t i = 0; i < vec_len(constant->gen_func.params); i++) {
                if (i != 0) fprintf(stream, ", ");
                hir_dump_type(constant->gen_func.params[i], stream);
            }
            fprintf(stream, ">");
            break;
    }
}

static void hir_expr_dump(HirExpr *expr, FILE *stream, size_t tabs) {
    fprintf(stream, "(\n");
    for (size_t i = 0; i < vec_len(expr->steps); i++) {
        HirExprStep *step = &expr->steps[i];
        ftabs(stream, tabs + 1);
        fprintf(stream, "step%lu: ", i);
        switch (step->kind) {
            case HIR_EXPR_STEP_CONST: hir_const_dump(&step->constant, stream); break;
            case HIR_EXPR_STEP_BINOP:
                fprintf(stream, "step%lu ", step->binop.ls);
                switch (step->binop.kind) {
                    case HIR_BINOP_ARITHMETIC:
                        switch (step->binop.arithm.kind) {
                            case HIR_BINOP_ARITHM_ADD: fprintf(stream, "+"); break;
                            case HIR_BINOP_ARITHM_SUB: fprintf(stream, "-"); break;
                            case HIR_BINOP_ARITHM_MUL: fprintf(stream, "*"); break;
                            case HIR_BINOP_ARITHM_DIV: fprintf(stream, "/"); break;
                        }
                        break;
                    case HIR_BINOP_ORDER:
                        switch (step->binop.order.kind) {
                            case HIR_BINOP_ORDER_LT: fprintf(stream, "<"); break;
                            case HIR_BINOP_ORDER_GT: fprintf(stream, ">"); break;
                            case HIR_BINOP_ORDER_LE: fprintf(stream, "<="); break;
                            case HIR_BINOP_ORDER_GE: fprintf(stream, ">="); break;
                        }
                        break;
                    case HIR_BINOP_COMPARE:
                        switch (step->binop.compare.kind) {
                            case HIR_COMPARE_EQ: fprintf(stream, "=="); break;
                            case HIR_COMPARE_NE: fprintf(stream, "!="); break;
                        }
                        break;
                    case HIR_BINOP_INT:
                        switch (step->binop.integer.kind) {
                            case HIR_BINOP_INT_MOD: fprintf(stream, "%%"); break;
                            case HIR_BINOP_INT_BITOR: fprintf(stream, "|"); break;
                            case HIR_BINOP_INT_BITAND: fprintf(stream, "&"); break;
                            case HIR_BINOP_INT_SHR: fprintf(stream, ">>"); break;
                            case HIR_BINOP_INT_SHL: fprintf(stream, "<<"); break;
                        }
                        break;
                    case HIR_BINOP_BOOL:
                        switch (step->binop.boolean.kind) {
                            case HIR_BINOP_BOOL_OR: fprintf(stream, "||"); break;
                            case HIR_BINOP_BOOL_AND: fprintf(stream, "&&"); break;
                              break;
                        }
                        break;
                }
                fprintf(stream, " step%lu", step->binop.rs);
                break;
            case HIR_EXPR_STEP_STRUCT_FIELD:
                fprintf(stream, "struct field %lu from step%lu", step->struct_field.idx, step->struct_field.step);
                break;
            case HIR_EXPR_STEP_GET_DECL: fprintf(stream, "decl%lu", step->decl_id); break;
            case HIR_EXPR_STEP_GET_LOCAL: fprintf(stream, "local%lu", step->decl_id); break;
            case HIR_EXPR_STEP_CALL:
                fprintf(stream, "step%lu(", step->call.callable);
                for (size_t i = 0; i < vec_len(step->call.args); i++) {
                    fprintf(stream, i == 0 ? "step%lu" : ", step%lu", step->call.args[i]);
                }
                fprintf(stream, ")");
                break;
            case HIR_EXPR_STEP_BOOL_SKIP:
                fprintf(stream, "if step%lu is %s skip with %s", step->bool_skip.condition,
                    step->bool_skip.expect ? "true" : "false", step->bool_skip.result ? "true" : "false");
                break;
            case HIR_EXPR_STEP_TAKE_REF: fprintf(stream, "take ref of step%lu", step->ref_step); break;
            case HIR_EXPR_STEP_DEREF: fprintf(stream, "dereference step%lu", step->ref_step); break;
            case HIR_EXPR_STEP_PTR_TO_INT:
                fprintf(stream, "cast ptr step%lu to ", step->ptr_to_int.step_id);
                hir_dump_type(step->ptr_to_int.type, stream);
                break;
            case HIR_EXPR_STEP_INT_TO_PTR:
                fprintf(stream, "cast int step%lu to ptr ", step->ptr_to_int.step_id);
                hir_dump_type(step->int_to_ptr.type, stream);
                break;
            case HIR_EXPR_STEP_CAST_INT:
                fprintf(stream, "cast int step%lu to ", step->cast_int.step_id);
                hir_dump_type(step->cast_int.dest, stream);
                break;
            case HIR_EXPR_STEP_NOT: fprintf(stream, "not step%lu", step->not_step); break;
            case HIR_EXPR_STEP_BUILD_STRUCT:
                hir_dump_type(step->build_struct.type, stream);
                fprintf(stream, " { ");
                for (size_t i = 0; i < vec_len(step->build_struct.fields); i++) {
                    fprintf(stream, i == 0 ? "step%lu" : ", step%lu", step->build_struct.fields[i]);
                }
                fprintf(stream, "}");
                break;
            case HIR_EXPR_STEP_BUILD_ARRAY:
                hir_dump_type(step->build_array.type, stream);
                fprintf(stream, " [ ");
                for (size_t i = 0; i < vec_len(step->build_array.elements); i++) {
                    fprintf(stream, i == 0 ? "step%lu" : ", step%lu", step->build_array.elements[i]);
                }
                fprintf(stream, "]");
                break;
            case HIR_EXPR_STEP_SIZEOF:
                hir_dump_type(step->size.type, stream);
                fprintf(stream, " size of ");
                hir_dump_type(step->size.type, stream);
                break;
            case HIR_EXPR_STEP_IDX_ARRAY:
                fprintf(stream, "idx step%lu of array step%lu", step->idx_array.idx, step->idx_array.value);
                break;
            case HIR_EXPR_STEP_IDX_POINTER:
                fprintf(stream, "idx step%lu of pointer step%lu", step->idx_pointer.idx, step->idx_pointer.value);
                break;
            case HIR_EXPR_STEP_NEG:
                fprintf(stream, "neg of step%lu", step->neg.step);
                break;
            case HIR_EXPR_STEP_CAST_PTR:
                fprintf(stream, "cast pointer step%lu to ", step->cast_ptr.step_id);
                hir_dump_type(step->cast_ptr.type, stream);
                break;
        }
        fprintf(stream, "\n");
    }
    ftabs(stream, tabs);
    fprintf(stream, ")");
}

static inline void hir_code_dump_tabbed(HirCode *code, FILE *stream, size_t tabs) {
    fprintf(stream, "{\n");
    tabs++;
    for (size_t i = 0; i < vec_len(code->stmts); i++) {
        ftabs(stream, tabs);
        HirStmt *stmt = &code->stmts[i];
        switch (stmt->kind) {
            case HIR_STMT_EXPR:
                fprintf(stream, "expr ");
                hir_expr_dump(&stmt->expr, stream, tabs);
                break;
            case HIR_STMT_RET:
                fprintf(stream, "return ");
                hir_expr_dump(&stmt->ret.value, stream, tabs);
                break;
            case HIR_STMT_STORE:
                fprintf(stream, "store ");
                hir_expr_dump(&stmt->store.rvalue, stream, tabs);
                fprintf(stream, " to ");
                hir_expr_dump(&stmt->store.lvalue, stream, tabs);
                break;
            case HIR_STMT_RET_VOID: fprintf(stream, "return"); break;
            case HIR_STMT_COND_JMP:
                fprintf(stream, "if [\n");
                tabs++;
                for (size_t i = 0; i < vec_len(stmt->cond_jmp.conds); i++) {
                    HirStmtCondJmpBlock *block = &stmt->cond_jmp.conds[i];
                    ftabs(stream, tabs + 1);
                    fprintf(stream, "case ");
                    hir_expr_dump(&block->cond, stream, tabs + 1);
                    fprintf(stream, " then ");
                    hir_code_dump_tabbed(block->code, stream, tabs + 1);
                    fprintf(stream, "\n");
                }
                tabs--;
                ftabs(stream, tabs);
                fprintf(stream, "]");
                if (stmt->cond_jmp.else_code) {
                    fprintf(stream, " else ");
                    hir_code_dump_tabbed(stmt->cond_jmp.else_code, stream, tabs);
                }
                break;
            case HIR_STMT_INIT_FINAL:
                fprintf(stream, "init final local%lu ", stmt->init_final.id);
                hir_expr_dump(&stmt->init_final.value, stream, tabs);
                break;
            case HIR_STMT_DECL_VAR: fprintf(stream, "decl var local%lu ", stmt->var_id); break;
            case HIR_STMT_LOOP:
                fprintf(stream, "loop loop%lu ", stmt->loop.id);
                hir_code_dump_tabbed(stmt->loop.code, stream, tabs);
                break;
            case HIR_STMT_BREAK: fprintf(stream, "break loop%lu ", stmt->break_loop.id); break;
            case HIR_STMT_CONTINUE: fprintf(stream, "continue loop%lu ", stmt->break_loop.id); break;
        }
        fprintf(stream, "\n");
    }
    tabs--;
    ftabs(stream, tabs);
    fprintf(stream, "}");
}

void hir_code_dump(HirCode *code, FILE *stream) {
    hir_code_dump_tabbed(code, stream, 0);
}

