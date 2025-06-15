#pragma once

#include "ast/node.h"
#include "parser/api.h"

AstValueInfo *parse_value_info(Parser *parser, bool is_public);
AstNode *parse_value_decl_node(Parser *parser, AstGlobal *global, bool is_public);
