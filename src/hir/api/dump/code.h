#pragma once

#include "hir/api/code.h"
#include "hir/api/const.h"
#include <stdbool.h>
#include <stdio.h>

void hir_code_dump(HirCode *code, FILE *stream);
void hir_const_dump(HirConst *constant, FILE *stream);

