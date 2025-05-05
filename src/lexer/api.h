#pragma once

#include "core/file_content.h"
#include "core/path.h"
#include "token.h"

typedef struct Lexer Lexer;

Lexer *lexer_from_file_content(const FileContent *content);
Lexer *lexer_from_file(const Path path);
Token lexer_next(Lexer *lexer);
void lexer_free(Lexer *lexer);
