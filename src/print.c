#include "print.h"
#include "core/log.h"
#include "core/attributes.h"
#include "core/file_content.h"
#include "core/slice.h"
#include "lexer/token.h"
#include "sema/module/value.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

static void log_cstr(va_list list) { printf("%s", va_arg(list, char*)); }
static void UNUSED log_int(va_list list UNUSED) { printf("%d", va_arg(list, int)); }
static void log_size(va_list list) { printf("%lu", va_arg(list, size_t)); }
static void UNUSED log_hex(va_list list UNUSED) { printf("%lx", va_arg(list, long)); }
static void log_errno(va_list list UNUSED) { printf("%s", strerror(errno)); }
static void log_slice(va_list list) {
    Slice arg = va_arg(list, Slice);
    fwrite(arg.value, 1, arg.length, stdout);
}


void setup_log() {
    log_register('s', log_cstr);
    log_register('S', log_slice);
    log_register('l', log_size);
    log_register('L', file_pos_print);
    log_register('v', sema_value_print);
    log_register('V', file_in_lines_view_print);
    log_register('T', token_print);
    log_register('E', log_errno);

}
