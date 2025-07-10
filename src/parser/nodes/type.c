#include "type.h"
#include "ast/type.h"
#include "core/keymap.h"
#include "core/mempool.h"
#include "core/null.h"
#include "lexer/token.h"
#include "parser/nodes/expr.h"
#include "parser/nodes/path.h"
#include "parser/parser.h"

AstType *parse_type(Parser *parser) {
    Token token = parser_take(parser);
    switch (token.kind) {
        case TOKEN_FUN: {
            PARSER_EXPECT_NEXT(parser, TOKEN_OPENING_CIRCLE_BRACE);
            AstType **args = vec_new_in(parser->mempool, AstType*);
            while (parser_next_is_not(parser, TOKEN_CLOSING_CIRCLE_BRACE)) {
                vec_push(args, NOT_NULL(parse_type(parser)));
                if (!parser_check_list_sep(parser, TOKEN_CLOSING_CIRCLE_BRACE)) return NULL;
            }
            Slice slice = slice_union(token.slice, parser_take(parser).slice);
            AstType *returns = parser_next_should_be(parser, TOKEN_FUN_RETURNS) ? NOT_NULL(parse_type(parser)) : NULL;
            if (returns) {
                slice = slice_union(slice, returns->slice);
            }
            return ast_type_new_function(parser->mempool, slice, args, returns);
        }
        case TOKEN_OPENING_CIRCLE_BRACE: {
            AstType *type = NOT_NULL(parse_type(parser));
            PARSER_EXPECT_NEXT(parser, TOKEN_CLOSING_CIRCLE_BRACE);
            return type;
        }
        case TOKEN_STRUCT: {
            PARSER_EXPECT_NEXT(parser, TOKEN_OPENING_FIGURE_BRACE);
            AstStructField *fields = keymap_new_in(parser->mempool, AstStructField);
            while (parser_next_is_not(parser, TOKEN_CLOSING_FIGURE_BRACE)) {
                bool is_public = parser_next_should_be(parser, TOKEN_PUBLIC);
                Slice name = PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice;
                PARSER_EXPECT_NEXT(parser, TOKEN_COLON);
                if (keymap_insert(fields, name, ast_struct_field_new(is_public, NOT_NULL(parse_type(parser))))) {
                    parser_err(parser, name, "duplicated field");
                }
                if (!parser_check_list_sep(parser, TOKEN_CLOSING_FIGURE_BRACE)) return NULL;
            }
            return ast_type_new_struct(parser->mempool, slice_union(token.slice, parser_take(parser).slice), fields);
        }
        case TOKEN_UNION: {
            PARSER_EXPECT_NEXT(parser, TOKEN_OPENING_FIGURE_BRACE);
            AstUnionField *variants = keymap_new_in(parser->mempool, AstUnionField);
            while (parser_next_is_not(parser, TOKEN_CLOSING_FIGURE_BRACE)) {
                bool is_public = parser_next_should_be(parser, TOKEN_PUBLIC);
                Slice name = PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice;
                PARSER_EXPECT_NEXT(parser, TOKEN_COLON);
                if (keymap_insert(variants, name, ast_union_field_new(is_public, NOT_NULL(parse_type(parser))))) {
                    parser_err(parser, name, "duplicated field");
                }
                if (!parser_check_list_sep(parser, TOKEN_CLOSING_FIGURE_BRACE)) return NULL;
            }
            return ast_type_new_union(parser->mempool, slice_union(token.slice, parser_take(parser).slice), variants);
        }
        case TOKEN_OPENING_SQUARE_BRACE: {
            AstExpr *length = NOT_NULL(parse_expr(parser));
            PARSER_EXPECT_NEXT(parser, TOKEN_CLOSING_SQUARE_BRACE);
            AstType *type = NOT_NULL(parse_type(parser));
            return ast_type_new_array(parser->mempool, slice_union(token.slice, type->slice), length, type);
        }
        case TOKEN_ENUM: {
            AstType *explicit = NULL;
            if (parser_next_should_be(parser, TOKEN_OPENING_CIRCLE_BRACE)) {
                explicit = NOT_NULL(parse_type(parser));
                PARSER_EXPECT_NEXT(parser, TOKEN_CLOSING_CIRCLE_BRACE);
            }
            PARSER_EXPECT_NEXT(parser, TOKEN_OPENING_FIGURE_BRACE);
            AstEnumVariant *variants = keymap_new_in(parser->mempool, AstEnumVariant);
            while (parser_next_is_not(parser, TOKEN_CLOSING_FIGURE_BRACE)) {
                Slice name = PARSER_EXPECT_NEXT(parser, TOKEN_IDENT).slice;
                AstExpr *expr = parser_next_should_be(parser, TOKEN_ASSIGN) ?
                    NOT_NULL(parse_expr(parser)) : NULL;
                if (keymap_insert(variants, name, ast_enum_variant_new(expr))) {
                    parser_err(parser, name, "duplicated field");
                }
                if (!parser_check_list_sep(parser, TOKEN_CLOSING_FIGURE_BRACE)) return NULL;
            }
            return ast_type_new_enum(parser->mempool, slice_union(token.slice, parser_take(parser).slice),
                explicit, variants);
        }
        case TOKEN_IDENT:
            parser_skip_next(parser);
            return ast_type_new_path(parser->mempool, NOT_NULL(parse_path(parser)));
        case TOKEN_STAR:
            return ast_type_new_pointer(parser->mempool, token.slice, NOT_NULL(parse_type(parser)));
        default:
            parser_err(parser, token.slice, "expected type");
            return NULL;
    }
}
