#include "api.h"
#include "core/assert.h"
#include "core/mempool.h"
#include "core/slice.h"
#include "lexer/lexer.h"
#include "lexer/token.h"
#include <stdio.h>

static inline bool char_is_digit(char c) {
    return c >= '0' && c <= '9';
}

static inline bool char_is_lower_latin(char c) {
    return (c >= 'a' && c <= 'z');
}

static inline bool char_is_upper_latin(char c) {
    return (c >= 'A' && c <= 'Z');
}

static inline bool char_is_latin(char c) {
    return char_is_lower_latin(c) || char_is_upper_latin(c);
}

static inline bool char_is_ident_start(char c) {
    return char_is_latin(c) || c == '_' || c == '$';
}

static inline bool char_is_ident(char c) {
    return char_is_ident_start(c) || char_is_digit(c);
}

static inline bool char_is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n';
}

static char lexer_peek_escaped_char(Lexer *lexer, char c, char brace) {
    if (c == EOF) {
        lexer_err(lexer, lexer->pos, "EOF while parsing string");
        return EOF;
    }
    if (c == '\\') {
        c = lexer_next_char(lexer);
        if (c == brace) {
            return brace;
        }
        switch (c) {
            case '0': return '\0';
            case 'n': return '\n';
            case 't': return '\t';
            case '\\': return '\\';
            default: {
                size_t old = lexer->start_pos;
                lexer->start_pos = lexer->pos - 1;
                lexer_err(lexer, lexer->pos, "invalid escape character");
                lexer->start_pos = old;
                break;
            }
        }
    }
    return c;
}

static inline bool char_is_based_digit(char c) {
    return char_is_digit(c) || char_is_latin(c);
}

static inline char char_to_digit(char c) {
    if (char_is_digit(c)) return c - '0';
    if (char_is_lower_latin(c)) return 10 + c - 'a';
    if (char_is_upper_latin(c)) return 10 + c - 'A';
    UNREACHABLE;
}

static Token lex_based_int(Lexer *lexer, char base) {
    uint64_t result = 0; 
    char c;
    while (char_is_based_digit(c = lexer_next_char(lexer))) {
        char digit = char_to_digit(c);
        if (digit >= base) {
            lexer->pos--;
            return token_integer(result);
        }
        result = result * base + digit;
    }
    lexer->pos--;
    return token_integer(result);
}

static Token lexer_try_next(Lexer *lexer) {
    char c;
    lexer_mark_parsed(lexer);
    while(char_is_whitespace(c = lexer_next_char(lexer))) {
        lexer_mark_parsed(lexer);
    }
    switch (c) {
        case '#':
            if (lexer_next_char_is(lexer, '`')) {
                while (true) {
                    char c = lexer_next_char(lexer);
                    if (c == EOF) {
                        lexer_err(lexer, lexer->pos, "unclosed multiline comment");
                        return token_simple(TOKEN_FAILED);
                    }
                    if (c == '`' && lexer_next_char_is(lexer, '#')) {
                        break;
                    }
                }
            } else {
                for (
                    char c = lexer_next_char(lexer);
                    c != '\n' && c != EOF;
                    c = lexer_next_char(lexer)
                );
            }
            return lexer_try_next(lexer);
        case '+':
            if (lexer_next_char_is(lexer, '=')) return token_simple(TOKEN_APPEND);
            return token_simple(TOKEN_PLUS);
        case '-':
            if (lexer_next_char_is(lexer, '=')) return token_simple(TOKEN_SUBTRACT);
            else if (lexer_next_char_is(lexer, '>')) return token_simple(TOKEN_FUN_RETURNS);
            return token_simple(TOKEN_MINUS);
        case '*':
            if (lexer_next_char_is(lexer, '=')) return token_simple(TOKEN_MULTIPLY);
            return token_simple(TOKEN_STAR);
        case '/':
            if (lexer_next_char_is(lexer, '=')) return token_simple(TOKEN_DIVIDE);
            return token_simple(TOKEN_SLASH);
        case '=':
            if (lexer_next_char_is(lexer, '=')) return token_simple(TOKEN_EQUALS);
            return token_simple(TOKEN_ASSIGN);
        case '!':
            if (lexer_next_char_is(lexer, '=')) return token_simple(TOKEN_NOT_EQUALS);
            return token_simple(TOKEN_NOT);
        case ':': return token_simple(TOKEN_COLON);
        case ';': return token_simple(TOKEN_SEMICOLON);
        case '%': return token_simple(TOKEN_MOD);
        case '&':
            if (lexer_next_char_is(lexer, '&')) return token_simple(TOKEN_AND);
            return token_simple(TOKEN_BITAND);
        case '|':
            if (lexer_next_char_is(lexer, '|')) return token_simple(TOKEN_OR);
            return token_simple(TOKEN_BITOR);
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
            else if (lexer_next_char_is(lexer, '<')) return token_simple(TOKEN_SHL);
            return token_simple(TOKEN_OPENING_ANGLE_BRACE);
        case '>':
            if (lexer_next_char_is(lexer, '=')) return token_simple(TOKEN_GREATER_EQ);
            else if (lexer_next_char_is(lexer, '>')) return token_simple(TOKEN_SHR);
            return token_simple(TOKEN_CLOSING_ANGLE_BRACE);
        case '\'': {
            c = lexer_peek_escaped_char(lexer, lexer_next_char(lexer), '\'');
            if (c == EOF) {
                return token_simple(TOKEN_FAILED);
            }
            if (lexer_next_char(lexer) != '\'') {
                lexer_err(lexer, lexer->pos, "unclosed character literal");
                return token_simple(TOKEN_FAILED);
            }
            return token_char(c);
        }
        case '\"': {
            char *string = vec_new_in(lexer->mempool, char);
            for (char c = lexer_next_char(lexer); c != '\"'; c = lexer_next_char(lexer)) {
                c = lexer_peek_escaped_char(lexer, c, '\"');
                if (c == EOF) {
                    return token_simple(TOKEN_FAILED);
                }
                vec_push(string, c);
            }
            return token_string(slice_new(string, vec_len(string)));
        }
        case EOF:
            return token_simple(TOKEN_EOI);
        default:
            if (char_is_digit(c)) {
                if (c == '0') {
                    if (lexer_next_char_is(lexer, 'x')) return lex_based_int(lexer, 16);
                    if (lexer_next_char_is(lexer, 'o')) return lex_based_int(lexer, 8);
                    if (lexer_next_char_is(lexer, 'b')) return lex_based_int(lexer, 2);
                }
                uint64_t value = c - '0';
                while (char_is_digit(c = lexer_next_char(lexer))) {
                    value = value * 10 + c - '0';
                }
                if (c == '.') {
                    uint64_t secondary = 0;
                    uint64_t sec_div = 1;
                    while (char_is_digit(c = lexer_next_char(lexer))) {
                        secondary = secondary * 10 + c - '0';
                        sec_div *= 10;
                    }
                    lexer->pos--;
                    return token_float(value + (long double)secondary / sec_div);
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
                else if (slice_eq(slice, slice_from_cstr("public"))) return token_simple(TOKEN_PUBLIC);
                else if (slice_eq(slice, slice_from_cstr("fun"))) return token_simple(TOKEN_FUN);
                else if (slice_eq(slice, slice_from_cstr("var"))) return token_simple(TOKEN_VAR);
                else if (slice_eq(slice, slice_from_cstr("final"))) return token_simple(TOKEN_FINAL);
                else if (slice_eq(slice, slice_from_cstr("const"))) return token_simple(TOKEN_CONST);
                else if (slice_eq(slice, slice_from_cstr("global"))) return token_simple(TOKEN_GLOBAL);
                else if (slice_eq(slice, slice_from_cstr("extern"))) return token_simple(TOKEN_EXTERN);
                else if (slice_eq(slice, slice_from_cstr("return"))) return token_simple(TOKEN_RETURN);
                else if (slice_eq(slice, slice_from_cstr("if"))) return token_simple(TOKEN_IF);
                else if (slice_eq(slice, slice_from_cstr("else"))) return token_simple(TOKEN_ELSE);
                else if (slice_eq(slice, slice_from_cstr("while"))) return token_simple(TOKEN_WHILE);
                else if (slice_eq(slice, slice_from_cstr("import"))) return token_simple(TOKEN_IMPORT);
                else if (slice_eq(slice, slice_from_cstr("as"))) return token_simple(TOKEN_AS);
                else if (slice_eq(slice, slice_from_cstr("true"))) return token_simple(TOKEN_TRUE);
                else if (slice_eq(slice, slice_from_cstr("false"))) return token_simple(TOKEN_FALSE);
                else if (slice_eq(slice, slice_from_cstr("continue"))) return token_simple(TOKEN_CONTINUE);
                else if (slice_eq(slice, slice_from_cstr("break"))) return token_simple(TOKEN_BREAK);
                else if (slice_eq(slice, slice_from_cstr("do"))) return token_simple(TOKEN_DO);
                else if (slice_eq(slice, slice_from_cstr("use"))) return token_simple(TOKEN_USE);
                else if (slice_eq(slice, slice_from_cstr("sizeof"))) return token_simple(TOKEN_SIZEOF);
                else if (slice_eq(slice, slice_from_cstr("defer"))) return token_simple(TOKEN_DEFER);
                else if (slice_eq(slice, slice_from_cstr("undefined"))) return token_simple(TOKEN_UNDEFINED);
                else if (slice_eq(slice, slice_from_cstr("for"))) return token_simple(TOKEN_FOR);
                else if (slice_eq(slice, slice_from_cstr("in"))) return token_simple(TOKEN_IN);
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
