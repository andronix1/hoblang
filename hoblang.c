#include "core/log.h"
#include "core/slice.h"
#include <stdio.h>

static void hob_log_cstr(va_list list) { printf("%s", va_arg(list, char*)); }
static void hob_log_int(va_list list) { printf("%d", va_arg(list, int)); }
static void hob_log_hex(va_list list) { printf("%lx", va_arg(list, long)); }
static void hob_log_slice(va_list list) {
    Slice arg = va_arg(list, Slice);
    fwrite(arg.value, 1, arg.length, stdout);
}

int main(int argc, const char **argv) {
    log_register('i', sizeof(int), hob_log_int);
    log_register('X', sizeof(long), hob_log_hex);
    log_register('S', sizeof(Slice), hob_log_slice);
    log_register('s', sizeof(const char*), hob_log_cstr);
    long a = 0xffffCcba321;
    logln("$$ $$1 $$2: $i, $X, '$S', '$s'", 123, a, slice_from_cstr("ASDSSDASAD"), "lol kek");
    return 0;
}
