#pragma once

#include "ir/api/const.h"
#include "ir/api/stmt/code.h"
#include <stdbool.h>
#include <stdio.h>

void ir_code_dump(IrCode *code, FILE *stream);
void ir_const_dump(IrConst *constant, FILE *stream);

