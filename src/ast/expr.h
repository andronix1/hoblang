#pragma once

#include "ast/api/path.h"
#include "ast/api/expr.h"
#include "ast/api/type.h"
#include "core/mempool.h"
#include "sema/module/api/value.h"
#include "sema/module/nodes/exprs/binop_ext.h"
#include <stdint.h>

typedef enum {
    AST_EXPR_PATH,
    AST_EXPR_INTEGER,
    AST_EXPR_STRING,
    AST_EXPR_CALL,
    AST_EXPR_SCOPE,
    AST_EXPR_BINOP,
    AST_EXPR_STRUCT,
} AstExprKind;

typedef struct {
    AstExpr *inner;
    AstExpr **args;
} AstCall;

typedef enum {
    AST_BINOP_ADD, AST_BINOP_SUBTRACT,
    AST_BINOP_MULTIPLY, AST_BINOP_DIVIDE,

    AST_BINOP_EQUALS, AST_BINOP_NOT_EQUALS,
    AST_BINOP_LESS, AST_BINOP_GREATER,
    AST_BINOP_LESS_EQ, AST_BINOP_GREATER_EQ,
} AstBinopKindKind;

typedef struct AstBinopKind {
    AstBinopKindKind kind;
    Slice slice;

    struct {
        SemaBinopKind kind;
        union {
            SemaBinopArithmeticKind arithmetic;
            SemaBinopCompareKind compare;
            SemaBinopEqKind eq;
        };
    } sema;
} AstBinopKind;

typedef struct {
    AstBinopKind kind;
    AstExpr *left, *right;
} AstBinop;

typedef struct {
    AstExpr *expr;

    struct {
        size_t field_idx;
    } sema;
} AstExprStructField;

typedef struct {
    AstType *type;
    AstExprStructField *fields_map;
} AstExprStructConstructor;

typedef struct AstExpr {
    AstExprKind kind;
    Slice slice;

    union {
        AstPath *path;
        AstExpr *scope;
        uint64_t integer;
        Slice string;
        AstCall call;
        AstBinop binop;
        AstExprStructConstructor structure;
    };

    struct {
        SemaValue *value;
    } sema;
} AstExpr;

bool ast_expr_eq(const AstExpr *a, const AstExpr *b);

static inline AstExprStructField ast_expr_struct_field_new(AstExpr *expr) {
    AstExprStructField field = {
        .expr = expr
    };
    return field;
}

static inline AstBinopKind ast_binop_kind_new(AstBinopKindKind kind, Slice slice) {
    AstBinopKind bkind = { .kind = kind, .slice = slice };
    return bkind;
}

AstExpr *ast_expr_new_path(Mempool *mempool, Slice slice, AstPath *path);
AstExpr *ast_expr_new_integer(Mempool *mempool, Slice slice, uint64_t integer);
AstExpr *ast_expr_new_string(Mempool *mempool, Slice slice, Slice string);
AstExpr *ast_expr_new_callable(Mempool *mempool, Slice slice, AstExpr *inner, AstExpr **args);
AstExpr *ast_expr_new_scope(Mempool *mempool, Slice slice, AstExpr *inner);
AstExpr *ast_expr_new_binop(Mempool *mempool, Slice slice, AstBinopKind kind, AstExpr *left, AstExpr *right);
AstExpr *ast_expr_new_struct(Mempool *mempool, Slice slice, AstType *type, AstExprStructField *fields_map);
