#include "lex.h"

#define TOKEN_PARSE(expr) \
	do { \
		LexOneErr err = (expr); \
		if (!err) return true; \
		lexer_rollback(lexer); \
		if (err == LEX_ONE_ERR) return true; \
	} while(0)

#define SYMBOL(c, type) TOKEN_PARSE(lex_symbol(lexer, c, type));
#define KEYWORD(s, type) TOKEN_PARSE(lex_keyword(lexer, s, type));
#define SYMBOL_DUAL(c1, c2, t1, t2) TOKEN_PARSE(lex_symbol_dual(lexer, c1, c2, t1, t2));

bool lex_next(Lexer *lexer) {
	lexer_skip_whitespace(lexer);
	lexer_begin(lexer);
	if (lexer_finished(lexer)) {
		return false;
	}
	lexer->token.location = lexer->location;
	
	SYMBOL('{', TOKEN_OPENING_FIGURE_BRACE);
	SYMBOL('}', TOKEN_CLOSING_FIGURE_BRACE);
	SYMBOL('(', TOKEN_OPENING_CIRCLE_BRACE);
	SYMBOL(')', TOKEN_CLOSING_CIRCLE_BRACE);
	SYMBOL('+', TOKEN_ADD);
	SYMBOL('-', TOKEN_MINUS);
	SYMBOL('*', TOKEN_MULTIPLY);
	SYMBOL('/', TOKEN_DIVIDE);
	SYMBOL(':', TOKEN_COLON);
	SYMBOL(';', TOKEN_SEMICOLON);
	SYMBOL(',', TOKEN_COMMA);
	SYMBOL_DUAL('=', '=', TOKEN_ASSIGN, TOKEN_EQUALS);
	SYMBOL_DUAL('>', '=', TOKEN_GREATER, TOKEN_GREATER_OR_EQUALS);
	SYMBOL_DUAL('<', '=', TOKEN_LESS, TOKEN_LESS_OR_EQUALS);
	KEYWORD("var", TOKEN_VAR);
	KEYWORD("fun", TOKEN_FUN);
	KEYWORD("if", TOKEN_IF);
	KEYWORD("else", TOKEN_ELSE);
	KEYWORD("return", TOKEN_RETURN);
	KEYWORD("extern", TOKEN_EXTERN);
	KEYWORD("module", TOKEN_MODULE);
	KEYWORD("as", TOKEN_AS);
	KEYWORD("true", TOKEN_TRUE);
	KEYWORD("false", TOKEN_FALSE);
	TOKEN_PARSE(lex_char(lexer));
	TOKEN_PARSE(lex_ident(lexer));
	TOKEN_PARSE(lex_integer(lexer));

	lex_err("unknown token");
	return true;
}
