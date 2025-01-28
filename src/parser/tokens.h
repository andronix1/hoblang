#pragma once

static inline bool token_semicolon_stop(TokenType type) { return type == TOKEN_SEMICOLON; }
static inline bool token_closing_circle_brace_stop(TokenType type) { return type == TOKEN_CLOSING_CIRCLE_BRACE; }
static inline bool token_opening_figure_brace_stop(TokenType type) { return type == TOKEN_OPENING_FIGURE_BRACE; }
static inline bool token_funcall_arg_stop(TokenType type) { return type == TOKEN_COMMA || type == TOKEN_CLOSING_CIRCLE_BRACE; }
static inline bool token_array_arg_stop(TokenType type) { return type == TOKEN_COMMA || type == TOKEN_CLOSING_FIGURE_BRACE; }
static inline bool token_idx_stop(TokenType type) { return type == TOKEN_CLOSING_SQUARE_BRACE; }
