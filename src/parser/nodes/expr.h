#pragma once

#include "ast/api/expr.h"
#include "parser/api.h"

AstExpr *parse_expr(Parser *parser);
