#pragma once

#include "core/mempool.h"
#include "ir/api/stmt/stmt.h"

typedef enum {
    IR_CODE_FLOW_PASSED,
    IR_CODE_FLOW_LOOP_BREAK,
    IR_CODE_FLOW_RETURN,
    IR_CODE_FLOW_UNREACHABLE,
} IrCodeFlow;

typedef struct IrCode {
    IrStmt **stmts;

    IrCodeFlow flow;
} IrCode;

IrCode *ir_code_new(Mempool *mempool, IrStmt **stmts);
