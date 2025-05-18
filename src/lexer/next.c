#include "api.h"
#include "core/mempool.h"
#include "core/slice.h"
#include "lexer/lexer.h"
#include "lexer/token.h"
#include <stdio.h>

static inline bool char_is_digit(char c) {
    return c >= '0' && c <= '9';
}

static inline bool char_is_ident_start(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '$';
}

static inline bool char_is_ident(char c) {
    return char_is_ident_start(c) || char_is_digit(c);
}

static inline bool char_is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n';
}

static Token lexer_try_next(Lexer *lexer) {
    char c;
    lexer_mark_parsed(lexer);
    while(char_is_whitespace(c = lexer_next_char(lexer))) {
        lexer_mark_parsed(lexer);
    }
    switch (c) {
        case '+':
            if (lexer_next_char_is(lexer, '=')) return token_simple(TOKEN_APPEND);
            return token_simple(TOKEN_PLUS);
        case '-':
            if (lexer_next_char_is(lexer, '=')) return token_simple(TOKEN_SUBTRACT);
            else if (lexer_next_char_is(lexer, '>')) return token_simple(TOKEN_FUN_RETURNS);
            return token_simple(TOKEN_MINUS);
        case '*': return token_simple(TOKEN_STAR);
        case '/': return token_simple(TOKEN_SLASH);
        case '=':
            if (lexer_next_char_is(lexer, '=')) return token_simple(TOKEN_EQUALS);
            return token_simple(TOKEN_ASSIGN);
        case '!':
            if (lexer_next_char_is(lexer, '=')) return token_simple(TOKEN_NOT_EQUALS);
            return token_simple(TOKEN_NOT);
        case ':': return token_simple(TOKEN_COLON);
        case ';': return token_simple(TOKEN_SEMICOLON);
        case '&': return token_simple(TOKEN_AND);
        case ',': return token_simple(TOKEN_COMMA);
        case '.': return token_simple(TOKEN_DOT);
        case '{': return token_simple(TOKEN_OPENING_FIGURE_BRACE);
        case '}': return token_simple(TOKEN_CLOSING_FIGURE_BRACE);
        case '(': return token_simple(TOKEN_OPENING_CIRCLE_BRACE);
        case ')': return token_simple(TOKEN_CLOSING_CIRCLE_BRACE);
        case '[': return token_simple(TOKEN_OPENING_SQUARE_BRACE);
        case ']': return token_simple(TOKEN_CLOSING_SQUARE_BRACE);
        case '<':
            if (lexer_next_char_is(lexer, '=')) return token_simple(TOKEN_LESS_EQ);
            return token_simple(TOKEN_OPENING_ANGLE_BRACE);
        case '>':
            if (lexer_next_char_is(lexer, '=')) return token_simple(TOKEN_GREATER_EQ);
            return token_simple(TOKEN_CLOSING_ANGLE_BRACE);
        case '\"': {
            char *string = vec_new_in(lexer->mempool, char);
            for (char c = lexer_next_char(lexer); c != '\"'; c = lexer_next_char(lexer)) {
                if (c == EOF) {
                    lexer_err(lexer, lexer->pos, "EOF while parsing string");
                    return token_simple(TOKEN_FAILED);
                }
                if (c == '\\') {
                    c = lexer_next_char(lexer);
                    switch (c) {
                        case '0': vec_push(string, '\0'); break;
                        case 'n': vec_push(string, '\n'); break;
                        case 't': vec_push(string, '\t'); break;
                        case '\"': vec_push(string, '\"'); break;
                        case '\\': vec_push(string, '\\'); break;
                        default: {
                            size_t old = lexer->start_pos;
                            lexer->start_pos = lexer->pos - 1;
                            lexer_err(lexer, lexer->pos, "invalid escape character");
                            lexer->start_pos = old;
                            break;
                        }
                    }
                } else vec_push(string, c);
            }
            return token_string(slice_new(string, vec_len(string)));
        }
        case EOF:
            return token_simple(TOKEN_EOI);
        default:
            if (char_is_digit(c)) {
                uint64_t value = c - '0';
                while (char_is_digit(c = lexer_next_char(lexer))) {
                    value = value * 10 + c - '0';
                }
                lexer->pos--;
                return token_integer(value);
            }
            if (char_is_ident_start(c)) {
                while (char_is_ident(lexer_next_char(lexer)));
                lexer->pos--;
                Slice slice = lexer_slice(lexer);
                if (slice_eq(slice, slice_from_cstr("type"))) return token_simple(TOKEN_TYPE);
                else if (slice_eq(slice, slice_from_cstr("struct"))) return token_simple(TOKEN_STRUCT);
                else if (slice_eq(slice, slice_from_cstr("local"))) return token_simple(TOKEN_LOCAL);
                else if (slice_eq(slice, slice_from_cstr("fun"))) return token_simple(TOKEN_FUN);
                else if (slice_eq(slice, slice_from_cstr("var"))) return token_simple(TOKEN_VAR);
                else if (slice_eq(slice, slice_from_cstr("final"))) return token_simple(TOKEN_FINAL);
                else if (slice_eq(slice, slice_from_cstr("const"))) return token_simple(TOKEN_CONST);
                else if (slice_eq(slice, slice_from_cstr("global"))) return token_simple(TOKEN_GLOBAL);
                else if (slice_eq(slice, slice_from_cstr("extern"))) return token_simple(TOKEN_EXTERN);
                else if (slice_eq(slice, slice_from_cstr("return"))) return token_simple(TOKEN_RETURN);
                else if (slice_eq(slice, slice_from_cstr("if"))) return token_simple(TOKEN_IF);
                else if (slice_eq(slice, slice_from_cstr("else"))) return token_simple(TOKEN_ELSE);
                else return token_simple(TOKEN_IDENT);
            }
            return token_simple(TOKEN_FAILED);
    }
}


Token lexer_next(Lexer *lexer) {
    Token result = lexer_try_next(lexer);
    if (result.kind == TOKEN_FAILED) {
        size_t sp = lexer->start_pos;
        size_t end = lexer->pos;
        while ((result = lexer_try_next(lexer)).kind == TOKEN_FAILED) {
            end = lexer->pos;
        }
        size_t spn = lexer->start_pos;
        lexer->start_pos = sp;
        lexer_err(lexer, end, "failed to recognize token");
        lexer->start_pos = spn;
    }
    result.slice = lexer_slice(lexer);
    lexer_mark_parsed(lexer);
    return result;
}
