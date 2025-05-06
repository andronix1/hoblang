#include "core/buffer.h"
#include "core/file_content.h"

typedef struct Lexer {
    const FileContent *content;
    bool content_ownership;

    Buffer buffer;
    size_t start_pos, pos;
} Lexer;

char lexer_next_char(Lexer *lexer);
bool lexer_next_char_is(Lexer *lexer, char c);
Slice lexer_slice(const Lexer *lexer);
void lexer_mark_parsed(Lexer *lexer);
void lexer_err(Lexer* lexer, size_t before, const char *fmt, ...);
