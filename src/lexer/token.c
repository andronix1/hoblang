#include "token.h"
#include "core/assert.h"
#include <stdio.h>

void token_print(va_list list) {
    Token token = va_arg(list, Token);
    switch (token.kind) {
        case TOKEN_MINUS: printf("`-`"); return;
        case TOKEN_PLUS: printf("`+`"); return;
        case TOKEN_APPEND: printf("`+=`"); return;
        case TOKEN_SUBTRACT: printf("`-=`"); return;
        case TOKEN_FUN_RETURNS: printf("`->`"); return;
        case TOKEN_ASSIGN: printf("`=`"); return;
        case TOKEN_OPENING_FIGURE_BRACE: printf("`{`"); return;
        case TOKEN_CLOSING_FIGURE_BRACE: printf("`}`"); return;
        case TOKEN_OPENING_CIRCLE_BRACE: printf("`(`"); return;
        case TOKEN_CLOSING_CIRCLE_BRACE: printf("`)`"); return;
        case TOKEN_COLON: printf("`:`"); return;
        case TOKEN_SEMICOLON: printf("`;`"); return;
        case TOKEN_COMMA: printf("`,`"); return;
        case TOKEN_DOT: printf("`.`"); return;
        case TOKEN_IDENT: printf("<ident>"); return;
        case TOKEN_INTEGER: printf("<integer>"); return;
        case TOKEN_STRUCT: printf("`struct`"); return;
        case TOKEN_LOCAL: printf("`local`"); return;
        case TOKEN_TYPE: printf("`type`"); return;
        case TOKEN_FUN: printf("`fun`"); return;
        case TOKEN_GLOBAL: printf("`global`"); return;
        case TOKEN_EXTERN: printf("`extern`"); return;
        case TOKEN_VAR: printf("`var`"); return;
        case TOKEN_FINAL: printf("`final`"); return;
        case TOKEN_CONST: printf("`const`"); return;
        case TOKEN_FAILED: printf("<fail>"); return;
        case TOKEN_EOI: printf("<end of input>"); return;
    }
    UNREACHABLE;
}
