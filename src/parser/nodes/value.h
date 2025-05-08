#pragma once

#include "ast/node.h"
#include "parser/api.h"

AstValueInfo *parse_value_info(Parser *parser, bool is_local);
AstNode *parse_value_decl_node(Parser *parser, bool is_local);
