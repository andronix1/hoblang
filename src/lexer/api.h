#pragma once

#include "core/file_content.h"
#include "token.h"

typedef struct Lexer Lexer;

Lexer *lexer_new(const FileContent *content, bool content_ownership);
Token lexer_next(Lexer *lexer);
void lexer_free(Lexer *lexer);
