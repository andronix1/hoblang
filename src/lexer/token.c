#include "token.h"
#include "core/assert.h"
#include <stdio.h>

void token_print(va_list list) {
    Token token = va_arg(list, Token);
    switch (token.kind) {
        case TOKEN_MINUS: printf("`-`"); return;
        case TOKEN_PLUS: printf("`+`"); return;
        case TOKEN_STAR: printf("`*`"); return;
        case TOKEN_MOD: printf("`%%`"); return;
        case TOKEN_SLASH: printf("`/`"); return;
        case TOKEN_DIVIDE: printf("`/=`"); return;
        case TOKEN_MULTIPLY: printf("`*=`"); return;
        case TOKEN_AND: printf("`&&`"); return;
        case TOKEN_OR: printf("`||`"); return;
        case TOKEN_APPEND: printf("`+=`"); return;
        case TOKEN_SUBTRACT: printf("`-=`"); return;
        case TOKEN_FUN_RETURNS: printf("`->`"); return;
        case TOKEN_ASSIGN: printf("`=`"); return;
        case TOKEN_OPENING_FIGURE_BRACE: printf("`{`"); return;
        case TOKEN_CLOSING_FIGURE_BRACE: printf("`}`"); return;
        case TOKEN_OPENING_CIRCLE_BRACE: printf("`(`"); return;
        case TOKEN_CLOSING_CIRCLE_BRACE: printf("`)`"); return;
        case TOKEN_OPENING_SQUARE_BRACE: printf("`[`"); return;
        case TOKEN_CLOSING_SQUARE_BRACE: printf("`]`"); return;
        case TOKEN_OPENING_ANGLE_BRACE: printf("`<`"); return;
        case TOKEN_CLOSING_ANGLE_BRACE: printf("`>`"); return;
        case TOKEN_COLON: printf("`:`"); return;
        case TOKEN_SEMICOLON: printf("`;`"); return;
        case TOKEN_COMMA: printf("`,`"); return;
        case TOKEN_DOT: printf("`.`"); return;
        case TOKEN_IDENT: printf("<ident>"); return;
        case TOKEN_CHAR: printf("<char>"); return;
        case TOKEN_INTEGER: printf("<integer>"); return;
        case TOKEN_STRING: printf("<string>"); return;
        case TOKEN_STRUCT: printf("`struct`"); return;
        case TOKEN_PUBLIC: printf("`public`"); return;
        case TOKEN_TYPE: printf("`type`"); return;
        case TOKEN_FUN: printf("`fun`"); return;
        case TOKEN_GLOBAL: printf("`global`"); return;
        case TOKEN_EXTERN: printf("`extern`"); return;
        case TOKEN_RETURN: printf("`return`"); return;
        case TOKEN_CONTINUE: printf("`continue`"); return;
        case TOKEN_BREAK: printf("`break`"); return;
        case TOKEN_VAR: printf("`var`"); return;
        case TOKEN_FINAL: printf("`final`"); return;
        case TOKEN_CONST: printf("`const`"); return;
        case TOKEN_IF: printf("`if`"); return;
        case TOKEN_ELSE: printf("`else`"); return;
        case TOKEN_WHILE: printf("`while`"); return;
        case TOKEN_IMPORT: printf("`import`"); return;
        case TOKEN_AS: printf("`as`"); return;
        case TOKEN_DO: printf("`do`"); return;
        case TOKEN_TRUE: printf("`true`"); return;
        case TOKEN_FALSE: printf("`false`"); return;
        case TOKEN_FAILED: printf("<fail>"); return;
        case TOKEN_EOI: printf("<end of input>"); return;
        case TOKEN_EQUALS: printf("`==`"); return;
        case TOKEN_NOT_EQUALS: printf("`!=`"); return;
        case TOKEN_GREATER_EQ: printf("`>=`"); return;
        case TOKEN_LESS_EQ: printf("`<=`"); return;
        case TOKEN_NOT: printf("`!`"); return;
        case TOKEN_BITAND: printf("`&`"); return;
        case TOKEN_BITOR: printf("`|`"); return;
    }
    UNREACHABLE;
}
