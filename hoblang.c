#include "core/file_content.h"
#include "core/log.h"
#include "core/slice.h"
#include "lexer/api.h"
#include "lexer/token.h"
#include "parser/api.h"
#include <stdio.h>

static void hob_log_cstr(va_list list) { printf("%s", va_arg(list, char*)); }
static void hob_log_int(va_list list) { printf("%d", va_arg(list, int)); }
static void hob_log_hex(va_list list) { printf("%lx", va_arg(list, long)); }
static void hob_log_slice(va_list list) {
    Slice arg = va_arg(list, Slice);
    fwrite(arg.value, 1, arg.length, stdout);
}

int main(int argc, const char **argv) {
    log_register('s', hob_log_cstr);
    log_register('S', hob_log_slice);
    log_register('L', file_pos_print);
    log_register('V', file_in_lines_view_print);
    log_register('T', token_print);
    Parser *parser = parser_new(lexer_new(file_content_new_in_memory(
        "type Super = struct {\n"
        "    a: i32,\n"
        "    b: mod.i32\n"
        "};\n"
        "local fun test() -> i32 {}"
    ), true), true);
    parser_parse(parser);
    parser_free(parser);
    return 0;
}
