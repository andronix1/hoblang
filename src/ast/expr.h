#pragma once

#include "ast/api/body.h"
#include "ast/api/path.h"
#include "ast/api/expr.h"
#include "ast/api/type.h"
#include "core/mempool.h"
#include <stdint.h>

typedef enum {
    AST_EXPR_PATH,
    AST_EXPR_INTEGER,
    AST_EXPR_CHAR,
    AST_EXPR_BOOL,
    AST_EXPR_FLOAT,
    AST_EXPR_STRING,
    AST_EXPR_CALL,
    AST_EXPR_SCOPE,
    AST_EXPR_BINOP,
    AST_EXPR_STRUCT,
    AST_EXPR_ARRAY,
    AST_EXPR_AS,
    AST_EXPR_TAKE_REF,
    AST_EXPR_NOT,
    AST_EXPR_NEG,
    AST_EXPR_IDX,
    AST_EXPR_INNER_PATH,
    AST_EXPR_FUNCTION,
    AST_EXPR_UNDEFINED,
} AstExprKind;

typedef struct {
    AstExpr *inner;
    AstExpr **args;
} AstCall;

typedef enum {
    AST_BINOP_ADD, AST_BINOP_SUBTRACT,
    AST_BINOP_MULTIPLY, AST_BINOP_DIVIDE,

    AST_BINOP_MOD,

    AST_BINOP_OR, AST_BINOP_AND,

    AST_BINOP_BITOR, AST_BINOP_BITAND,
    AST_BINOP_SHL, AST_BINOP_SHR,

    AST_BINOP_EQUALS, AST_BINOP_NOT_EQUALS,
    AST_BINOP_LESS, AST_BINOP_GREATER,
    AST_BINOP_LESS_EQ, AST_BINOP_GREATER_EQ,
} AstBinopKindKind;

typedef struct AstBinopKind {
    AstBinopKindKind kind;
    Slice slice;
} AstBinopKind;

typedef struct {
    AstType *type;
    AstExpr *inner;
    Slice slice;
} AstAs;

typedef struct {
    AstType *type;
    AstExpr **elements;
} AstExprArray;

typedef struct {
    AstBinopKind kind;
    AstExpr *left, *right;
} AstBinop;

typedef struct {
    AstExpr *expr;
} AstExprStructField;

typedef struct {
    AstType *type;
    AstExprStructField *fields_map;
} AstExprStructConstructor;

typedef struct {
    AstExpr *inner;
    AstPath *path;
} AstExprInnerPath;

typedef struct {
    Slice name;
    AstType *type;
} AstExprFuncArg;

static inline AstExprFuncArg ast_expr_func_arg_new(Slice name, AstType *type) {
    AstExprFuncArg arg = {
        .name = name,
        .type = type,
    };
    return arg;
}

typedef struct {
    AstExprFuncArg *args;
    AstType *returns;
    AstBody *body;
} AstExprFunc;

typedef struct {
    AstExpr *inner;
    AstExpr *idx;
} AstExprIdx;

typedef struct AstExpr {
    AstExprKind kind;
    Slice slice;

    union {
        AstPath *path;
        AstExpr *scope;
        AstExpr *not_inner;
        AstExpr *neg_inner;
        AstExpr *take_ref_inner;
        AstExprArray array;
        AstExprFunc func;
        AstExprIdx idx;
        AstExprInnerPath inner_path;
        uint64_t integer;
        long double float_value;
        char character;
        bool boolean;
        Slice string;
        AstCall call;
        AstAs as;
        AstBinop binop;
        AstExprStructConstructor structure;
    };
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

AstExpr *ast_expr_new_undefined(Mempool *mempool, Slice slice);
AstExpr *ast_expr_new_char(Mempool *mempool, Slice slice, char c);
AstExpr *ast_expr_new_bool(Mempool *mempool, Slice slice, bool value);
AstExpr *ast_expr_new_path(Mempool *mempool, Slice slice, AstPath *path);
AstExpr *ast_expr_new_array(Mempool *mempool, Slice slice, AstType *type, AstExpr **elements);
AstExpr *ast_expr_new_function(Mempool *mempool, Slice slice, AstExprFuncArg *args, AstType *returns, AstBody *body);
AstExpr *ast_expr_new_inner_path(Mempool *mempool, Slice slice, AstExpr *inner, AstPath *path);
AstExpr *ast_expr_new_take_ref(Mempool *mempool, Slice slice, AstExpr *inner);
AstExpr *ast_expr_new_not(Mempool *mempool, Slice slice, AstExpr *inner);
AstExpr *ast_expr_new_neg(Mempool *mempool, Slice slice, AstExpr *inner);
AstExpr *ast_expr_new_idx(Mempool *mempool, Slice slice, AstExpr *inner, AstExpr *idx);
AstExpr *ast_expr_new_as(Mempool *mempool, Slice slice, Slice as_slice, AstExpr *inner, AstType *as);
AstExpr *ast_expr_new_integer(Mempool *mempool, Slice slice, uint64_t integer);
AstExpr *ast_expr_new_float(Mempool *mempool, Slice slice, long double float_value);
AstExpr *ast_expr_new_string(Mempool *mempool, Slice slice, Slice string);
AstExpr *ast_expr_new_callable(Mempool *mempool, Slice slice, AstExpr *inner, AstExpr **args);
AstExpr *ast_expr_new_scope(Mempool *mempool, Slice slice, AstExpr *inner);
AstExpr *ast_expr_new_binop(Mempool *mempool, Slice slice, AstBinopKind kind, AstExpr *left, AstExpr *right);
AstExpr *ast_expr_new_struct(Mempool *mempool, Slice slice, AstType *type, AstExprStructField *fields_map);
