#include <stdarg.h>
#include "../ast.h"

void print_ast_binop_type(FILE *stream, va_list list) {
	AstBinopType binop = va_arg(list, AstBinopType);
	const char *strs[] = {
		[AST_BINOP_ADD] = "+",
		[AST_BINOP_SUB] = "-",
		[AST_BINOP_MUL] = "*",
		[AST_BINOP_BITAND] = "&",
		[AST_BINOP_BITOR] = "|",
		[AST_BINOP_SHR] = ">>",
		[AST_BINOP_SHL] = "<<",
		[AST_BINOP_XOR] = "^",
		[AST_BINOP_DIV] = "/",
		[AST_BINOP_EQ] = "==",
		[AST_BINOP_NEQ] = "!=",
		[AST_BINOP_GT] = ">",
		[AST_BINOP_GE] = ">=",
		[AST_BINOP_LT] = "<",
		[AST_BINOP_LE] = "<="
	};
	print_to(stream, strs[binop]);
}

void print_ast_mod_path(FILE *stream, va_list list) {
	AstModPath *path = va_arg(list, AstModPath*);
	for (size_t i = 0; i < vec_len(path->segments); i++) {
		print_to(stream, i == 0 ? "{slice}" : "::{slice}", &path->segments[i]);
	}
}

void print_ast_value(FILE *stream, va_list list) {
	AstValue *value = va_arg(list, AstValue*);
	print_to(stream, "{ast::path}", &value->mod_path);
	for (size_t i = 0; i < vec_len(value->segments); i++) {
		AstValueSegment *seg = &value->segments[i];
		switch (seg->type) {
			case AST_VALUE_IDENT: print_to(stream, ".{slice}", &seg->ident); break;
			case AST_VALUE_DEREF: print_to(stream, ".*"); break;
			case AST_VALUE_IDX: print_to(stream, "[{ast::expr}]", seg->idx); break;
		}
	}
}

void print_ast_expr(FILE *stream, va_list list) {
	AstExpr *expr = va_arg(list, AstExpr*);
	switch (expr->type) {
		case AST_EXPR_UNARY: {
			switch (expr->unary.type) {
				case AST_UNARY_MINUS: print_to(stream, "-{ast::expr}", expr->unary.expr); break;
				case AST_UNARY_BITNOT: print_to(stream, "~{ast::expr}", expr->unary.expr); break;
			}
			break;
		}
		case AST_EXPR_REF: print_to(stream, "&{ast::expr}", &expr->value); break;
		case AST_EXPR_NOT: print_to(stream, "!({ast::val})", &expr->value); break;
		case AST_EXPR_VALUE: print_to(stream, "{ast::val}", &expr->value); break;
		case AST_EXPR_INTEGER: print_to(stream, "{long}", expr->integer); break;
		case AST_EXPR_CHAR: fprintf(stream, "'%c'", expr->character); break;
		case AST_EXPR_BOOL: print_to(stream, "{bool}", &expr->boolean); break;
		case AST_EXPR_BINOP: print_to(stream, "({ast::expr} {ast::binop} {ast::expr})", expr->binop.left, expr->binop.type, expr->binop.right); break;
		/* case AST_EXPR_ARRAY:
			print_to(stream, "[");
			for (size_t i = 0; i < vec_len(expr->array); i++) {
				print_to(stream, i == 0 ? "{ast::expr}" : ", {ast::expr}", &expr->array[i]);
			}
			print_to(stream, "]");
			break;
		*/
		case AST_EXPR_STR:
			print_to(stream, "\"{slice}\"", &expr->str);
			break;
		case AST_EXPR_FUNCALL:
			print_to(stream, "{ast::val}(", &expr->func_call.value);
			for (size_t i = 0; i < vec_len(expr->func_call.args); i++) {
				if (i != 0) print_to(stream, ", ");
				print_to(stream, "{ast::expr}", &expr->func_call.args[i]);
			}
			print_to(stream, ")");
			break;
		case AST_EXPR_AS:
			print_to(stream, "{ast::expr} as {ast::type}", expr->as.expr, &expr->as.type);
			break;
	}
}
