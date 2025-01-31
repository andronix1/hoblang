#pragma once

#include <stdarg.h>
#include <stdio.h>

void print_ast_mod_path(FILE *stream, va_list list);
void print_ast_expr(FILE *stream, va_list list);
void print_ast_binop_type(FILE *stream, va_list list);
void print_ast_type(FILE *stream, va_list list);
void print_ast_path(FILE *stream, va_list list);
void print_ast_inner_path(FILE *stream, va_list list);
void print_ast_decl_path(FILE *stream, va_list list);
