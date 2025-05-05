#include "core/file_content.h"
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
    log_register('V', sizeof(const char*), file_in_lines_view_print);
    const char *str = "hello, bro, how are you?\n    i am very cool, lol\nyes, very cool\n";
    FileContent *content = file_content_new_in_memory(str);
    logln("$V", file_content_get_in_lines_view(content, slice_new(&str[30], 25)));
    file_content_free(content);
    return 0;
}
