#include "core/file_content.h"
#include "core/log.h"
#include "core/slice.h"
#include "lexer/api.h"
#include "lexer/token.h"
#include "parser/api.h"
#include "sema/module/api.h"
#include "sema/module/api/type.h"
#include "llvm/module/api.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

static void hob_log_cstr(va_list list) { printf("%s", va_arg(list, char*)); }
static void hob_log_int(va_list list) { printf("%d", va_arg(list, int)); }
static void hob_log_size(va_list list) { printf("%lu", va_arg(list, size_t)); }
static void hob_log_hex(va_list list) { printf("%lx", va_arg(list, long)); }
static void log_errno(va_list list) { printf("%s", strerror(errno)); }
static void hob_log_slice(va_list list) {
    Slice arg = va_arg(list, Slice);
    fwrite(arg.value, 1, arg.length, stdout);
}

int main(int argc, char **argv) {
    log_register('s', hob_log_cstr);
    log_register('S', hob_log_slice);
    log_register('l', hob_log_size);
    log_register('L', file_pos_print);
    log_register('V', file_in_lines_view_print);
    log_register('T', token_print);
    log_register('t', sema_type_print);
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
    LlvmModule *module = llvm_module_new(sema_module_new(parser_new(lexer_new(content))));
    llvm_module_read(module);
    llvm_module_emit(module);
    if (!llvm_module_write_obj(module, "test.o")) {
        logln("failed to write module");
        return 1;
    }
    llvm_module_free(module);
    return 0;
}
