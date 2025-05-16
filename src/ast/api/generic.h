#pragma once

#include <stdbool.h>

typedef struct AstGeneric AstGeneric;
typedef struct AstGenericBuilder AstGenericBuilder;

bool ast_generic_builder_eq(const AstGenericBuilder *a, const AstGenericBuilder *b);
