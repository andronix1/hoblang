#pragma once

typedef enum {
    SEMA_BINOP_EQ_INT,
    SEMA_BINOP_EQ_BOOL,
} SemaBinopEqKind;

typedef enum {
    SEMA_BINOP_COMPARE_INT,
    SEMA_BINOP_COMPARE_UINT,
} SemaBinopCompareKind;

typedef enum {
    SEMA_BINOP_ARITHMETIC_INT,
    SEMA_BINOP_ARITHMETIC_UINT,
} SemaBinopArithmeticKind;

typedef enum {
    SEMA_BINOP_ARITHMETIC,
    SEMA_BINOP_EQ,
    SEMA_BINOP_COMPARE,
} SemaBinopKind;
