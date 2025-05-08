#pragma once

#include "ast/api/node.h"
#include "ast/api/path.h"
#include "ast/api/type.h"
#include "core/mempool.h"

extern Mempool *mempool;

AstPath *create_path(const char *str);
AstType *create_type(const char *str);
void check_parsing(AstNode **expects, const char *code);

int on_test_setup();
int on_test_end();
