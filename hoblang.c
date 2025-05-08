#include "core/file_content.h"
#include "core/log.h"
#include "core/slice.h"
#include "lexer/api.h"
#include "lexer/token.h"
#include "parser/api.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

static void hob_log_cstr(va_list list) { printf("%s", va_arg(list, char*)); }
static void hob_log_int(va_list list) { printf("%d", va_arg(list, int)); }
static void hob_log_hex(va_list list) { printf("%lx", va_arg(list, long)); }
static void log_errno(va_list list) { printf("%s", strerror(errno)); }
static void hob_log_slice(va_list list) {
    Slice arg = va_arg(list, Slice);
    fwrite(arg.value, 1, arg.length, stdout);
}

int main(int argc, char **argv) {
    log_register('s', hob_log_cstr);
    log_register('S', hob_log_slice);
    log_register('L', file_pos_print);
    log_register('V', file_in_lines_view_print);
    log_register('T', token_print);
    log_register('E', log_errno);
    if (argc != 2) {
        logln("usage: $s <path>", argv[0]);
        return 1;
    }
    FileContent *content = file_content_read(argv[1]);
    if (!content) {
        logln("failed to read file content: $E");
        return 1;
    }
    Parser *parser = parser_new(lexer_new(content, true), true);
    parser_parse(parser);
    parser_free(parser);
    return 0;
}
