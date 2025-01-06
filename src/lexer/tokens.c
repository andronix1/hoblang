#include "tokens.h"

TokenType token_type(Token *token) {
	return token == NULL ? TOKEN_EOI : token->type;
}

int printf_arginfo_token(const struct printf_info *info __attribute__((unused)), size_t n, int *argtypes, int *size) {
	if (n > 0) {
		*argtypes = PA_POINTER;
		*size = sizeof(Token*);
	}
	return 1;
}

int printf_output_token(FILE *stream, const struct printf_info *info __attribute__((unused)), const void *const *args) {	
	const Token *token = *(Token**)*args;
	if (token == NULL || token->type == TOKEN_EOI) {
		return fprintf(stream, "<EOF>");
	}
	switch (token->type) {
    	case TOKEN_EXTERN: return fprintf(stream, "extern");
    	case TOKEN_AS: return fprintf(stream, "as");
    	case TOKEN_FUN: return fprintf(stream, "fun");
    	case TOKEN_VAR: return fprintf(stream, "var");
		case TOKEN_IF: return fprintf(stream, "if");
    	case TOKEN_ELSE: return fprintf(stream, "else");
    	case TOKEN_TRUE: return fprintf(stream, "true");
    	case TOKEN_FALSE: return fprintf(stream, "false");
		case TOKEN_COLON: return fprintf(stream, ":");
    	case TOKEN_COMMA: return fprintf(stream, ",");
    	case TOKEN_SEMICOLON: return fprintf(stream, ";");
    	case TOKEN_ASSIGN: return fprintf(stream, "=");
		case TOKEN_OPENING_CIRCLE_BRACE: return fprintf(stream, "(");
		case TOKEN_CLOSING_CIRCLE_BRACE: return fprintf(stream, ")");
		case TOKEN_OPENING_FIGURE_BRACE: return fprintf(stream, "{");
		case TOKEN_CLOSING_FIGURE_BRACE: return fprintf(stream, "}");
		case TOKEN_ADD: return fprintf(stream, "+");
    	case TOKEN_MINUS: return fprintf(stream, "-");
    	case TOKEN_MULTIPLY: return fprintf(stream, "*");
    	case TOKEN_DIVIDE: return fprintf(stream, "/");
    	case TOKEN_EQUALS: return fprintf(stream, "==");
    	case TOKEN_NOT_EQUALS: return fprintf(stream, "!=");
    	case TOKEN_LESS: return fprintf(stream, "<");
    	case TOKEN_LESS_OR_EQUALS: return fprintf(stream, "<=");
    	case TOKEN_GREATER: return fprintf(stream, ">");
    	case TOKEN_GREATER_OR_EQUALS: return fprintf(stream, ">=");
    	case TOKEN_INTEGER: return fprintf(stream, "%ld", token->integer);
    	case TOKEN_CHAR: return fprintf(stream, "'%c'", token->character);
    	case TOKEN_IDENT: return fatptr_print_to(&token->ident, stream);
		default: return fprintf(stream, "<unknown type = %lx>", token->type);
	}
}

void token_register_printf() {
	register_printf_specifier('T', printf_output_token, printf_arginfo_token);
}
