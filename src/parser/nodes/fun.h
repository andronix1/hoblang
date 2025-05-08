#pragma once

#include "ast/api/global.h"
#include "ast/api/fun.h"
#include "parser/api.h"

AstFunInfo *parse_fun_info(Parser *parser, bool is_local);
AstNode *parse_fun_decl_node(Parser *parser, AstGlobal *global, bool is_local);
