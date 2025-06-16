#include "code.h"
#include "ir/const.h"
#include "ir/stmt/code.h"
#include "ir/stmt/stmt.h"
#include <stdio.h>

static inline void ftabs(FILE *stream, size_t tabs) {
    for (size_t i = 0; i < tabs; i++) fprintf(stream, "  ");
}

void ir_const_dump(IrConst *constant, FILE *stream) {
    switch (constant->kind) {
        case IR_CONST_BOOL: fprintf(stream, constant->boolean ? "true" : "false"); break;
        case IR_CONST_INT: fprintf(stream, "int of type type%lu %lu", constant->type, constant->integer); break;
        case IR_CONST_REAL: fprintf(stream, "real of type type%lu %Lf", constant->type, constant->real); break;
        case IR_CONST_STRUCT:
            fprintf(stream, "type%lu { ", constant->type);
            for (size_t i = 0; i < vec_len(constant->struct_fields); i++) {
                if (i == 0) fprintf(stream, ", ");
                ir_const_dump(constant->struct_fields[i], stream);
            }
            fprintf(stream, "}");
            break;
        case IR_CONST_DECL_PTR: fprintf(stream, "const pointer to decl%lu", constant->decl); break;
    }
}

static void ir_expr_dump(IrExpr *expr, FILE *stream, size_t tabs) {
    fprintf(stream, "(\n");
    for (size_t i = 0; i < vec_len(expr->steps); i++) {
        IrExprStep *step = &expr->steps[i];
        ftabs(stream, tabs + 1);
        fprintf(stream, "step%lu: ", i);
        switch (step->kind) {
            case IR_EXPR_STEP_STRING: fprintf(stream, "<string>"); break;
            case IR_EXPR_STEP_CONST: ir_const_dump(step->constant, stream); break;
            case IR_EXPR_STEP_BINOP:
                fprintf(stream, "step%lu ", step->binop.ls);
                switch (step->binop.kind) {
                    case IR_BINOP_ARITHMETIC:
                        switch (step->binop.arithm.kind) {
                            case IR_BINOP_ARITHM_ADD: fprintf(stream, "+"); break;
                            case IR_BINOP_ARITHM_SUB: fprintf(stream, "-"); break;
                            case IR_BINOP_ARITHM_MUL: fprintf(stream, "*"); break;
                            case IR_BINOP_ARITHM_DIV: fprintf(stream, "/"); break;
                        }
                        break;
                    case IR_BINOP_ORDER:
                        switch (step->binop.order.kind) {
                            case IR_BINOP_ORDER_LT: fprintf(stream, "<"); break;
                            case IR_BINOP_ORDER_GT: fprintf(stream, ">"); break;
                            case IR_BINOP_ORDER_LE: fprintf(stream, "<="); break;
                            case IR_BINOP_ORDER_GE: fprintf(stream, ">="); break;
                        }
                        break;
                    case IR_BINOP_COMPARE:
                        switch (step->binop.compare.kind) {
                            case IR_COMPARE_EQ: fprintf(stream, "=="); break;
                            case IR_COMPARE_NE: fprintf(stream, "!="); break;
                        }
                        break;
                    case IR_BINOP_INT:
                        switch (step->binop.integer.kind) {
                            case IR_BINOP_INT_MOD: fprintf(stream, "%%"); break;
                            case IR_BINOP_INT_BITOR: fprintf(stream, "|"); break;
                            case IR_BINOP_INT_BITAND: fprintf(stream, "&"); break;
                            case IR_BINOP_INT_SHR: fprintf(stream, ">>"); break;
                            case IR_BINOP_INT_SHL: fprintf(stream, "<<"); break;
                        }
                        break;
                    case IR_BINOP_BOOL:
                        switch (step->binop.boolean.kind) {
                            case IR_BINOP_BOOL_OR: fprintf(stream, "||"); break;
                            case IR_BINOP_BOOL_AND: fprintf(stream, "&&"); break;
                              break;
                        }
                        break;
                }
                fprintf(stream, " step%lu", step->binop.rs);
                break;
            case IR_EXPR_STEP_STRUCT_FIELD:
                fprintf(stream, "struct field %lu from step%lu", step->struct_field.idx, step->struct_field.step);
                break;
            case IR_EXPR_STEP_GET_DECL: fprintf(stream, "decl%lu", step->decl_id); break;
            case IR_EXPR_STEP_GET_LOCAL: fprintf(stream, "local%lu", step->decl_id); break;
            case IR_EXPR_STEP_CALL:
                fprintf(stream, "step%lu(", step->call.callable);
                for (size_t i = 0; i < vec_len(step->call.args); i++) {
                    fprintf(stream, i == 0 ? "step%lu" : ", step%lu", step->call.args[i]);
                }
                fprintf(stream, ")");
                break;
            case IR_EXPR_STEP_BOOL_SKIP:
                fprintf(stream, "if step%lu is %s skip with %s", step->bool_skip.condition,
                    step->bool_skip.expect ? "true" : "false", step->bool_skip.result ? "true" : "false");
                break;
            case IR_EXPR_STEP_TAKE_REF: fprintf(stream, "take ref of step%lu", step->ref_step); break;
            case IR_EXPR_STEP_DEREF: fprintf(stream, "dereference step%lu", step->ref_step); break;
            case IR_EXPR_STEP_CAST_INT:
                fprintf(stream, "cast int step%lu to type%lu", step->cast_int.step_id, step->cast_int.dest);
                break;
            case IR_EXPR_STEP_NOT: fprintf(stream, "not step%lu", step->not_step); break;
            case IR_EXPR_STEP_BUILD_STRUCT:
                fprintf(stream, "type%lu { ", step->build_struct.type);
                for (size_t i = 0; i < vec_len(step->build_struct.fields); i++) {
                    fprintf(stream, i == 0 ? "step%lu" : ", step%lu", step->build_struct.fields[i]);
                }
                fprintf(stream, "}");
                break;
            case IR_EXPR_STEP_SIZEOF:
                fprintf(stream, "size of type type%lu of type%lu", step->size.of, step->size.type);
                break;
        }
        fprintf(stream, "\n");
    }
    ftabs(stream, tabs);
    fprintf(stream, ")");
}

static inline void ir_code_dump_tabbed(IrCode *code, FILE *stream, size_t tabs) {
    fprintf(stream, "{\n");
    tabs++;
    for (size_t i = 0; i < vec_len(code->stmts); i++) {
        ftabs(stream, tabs);
        IrStmt *stmt = code->stmts[i];
        switch (stmt->kind) {
            case IR_STMT_EXPR:
                fprintf(stream, "expr ");
                ir_expr_dump(&stmt->expr, stream, tabs);
                break;
            case IR_STMT_RET:
                fprintf(stream, "return ");
                ir_expr_dump(&stmt->ret.value, stream, tabs);
                break;
            case IR_STMT_STORE:
                fprintf(stream, "store ");
                ir_expr_dump(&stmt->store.rvalue, stream, tabs);
                fprintf(stream, " to ");
                ir_expr_dump(&stmt->store.lvalue, stream, tabs);
                break;
            case IR_STMT_RET_VOID: fprintf(stream, "return"); break;
            case IR_STMT_COND_JMP:
                fprintf(stream, "if [\n");
                tabs++;
                for (size_t i = 0; i < vec_len(stmt->cond_jmp.conds); i++) {
                    IrStmtCondJmpBlock *block = &stmt->cond_jmp.conds[i];
                    ftabs(stream, tabs + 1);
                    fprintf(stream, "case ");
                    ir_expr_dump(&block->cond, stream, tabs + 1);
                    fprintf(stream, " then ");
                    ir_code_dump_tabbed(block->code, stream, tabs + 1);
                    fprintf(stream, "\n");
                }
                tabs--;
                ftabs(stream, tabs);
                fprintf(stream, "]");
                if (stmt->cond_jmp.else_code) {
                    fprintf(stream, " else ");
                    ir_code_dump_tabbed(stmt->cond_jmp.else_code, stream, tabs);
                }
                break;
            case IR_STMT_INIT_FINAL:
                fprintf(stream, "init final local%lu ", stmt->init_final.id);
                ir_expr_dump(&stmt->init_final.value, stream, tabs);
                break;
            case IR_STMT_DECL_VAR: fprintf(stream, "decl var local%lu ", stmt->var_id); break;
            case IR_STMT_LOOP:
                fprintf(stream, "loop loop%lu ", stmt->loop.id);
                ir_code_dump_tabbed(stmt->loop.code, stream, tabs);
                break;
            case IR_STMT_BREAK: fprintf(stream, "break loop%lu ", stmt->break_loop.id); break;
            case IR_STMT_CONTINUE: fprintf(stream, "continue loop%lu ", stmt->break_loop.id); break;
        }
        fprintf(stream, "\n");
    }
    tabs--;
    ftabs(stream, tabs);
    fprintf(stream, "}");
}

void ir_code_dump(IrCode *code, FILE *stream) {
    ir_code_dump_tabbed(code, stream, 0);
}

