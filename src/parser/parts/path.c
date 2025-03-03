#include "ast/private/path.h"
#include "lexer/token.h"
#include "parser/private.h"

bool parse_path(Parser *parser, AstPath *path) {
    path->segments = vec_new(AstPathSegment);
    while (true) {
		Token *token = parser_next(parser);
		AstPathSegment segment;
        segment.loc = token->location;
		switch (token->type) {
			case TOKEN_NULL:
				segment.type = AST_PATH_SEG_NULL;
				break;
			case TOKEN_IDENT:
				segment.type = AST_PATH_SEG_IDENT;
				segment.ident = token->ident;
				break;
            case TOKEN_SIZEOF:
				segment.type = AST_PATH_SEG_SIZEOF;
                break;
			case TOKEN_MULTIPLY:
				segment.type = AST_PATH_SEG_DEREF;
				break;
			default:
				PARSE_ERROR(EXPECTED("inner path segment"));
				return false;
		}
        path->segments = vec_push(path->segments, &segment);
        if (parser_next_is_not(parser, TOKEN_DOT)) {
            return true;
        }
    }
}
