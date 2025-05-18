#include "log.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static HobLogHandleFunc print_handles[1 << (sizeof(char) * 8)] = { NULL };

void log_register(char id, HobLogHandleFunc func) {
    assert(print_handles[(size_t)id] == NULL);
    print_handles[(size_t)id] = func;
}

void logv(const char *fmt, va_list list) {
    size_t len = strlen(fmt);
    for (size_t i = 0; i < len; i++) {
        if (fmt[i] == '$') {
            char id = fmt[i + 1];
            if (id == '$') {
                putchar('$');
                i++;
                continue;
            }
            HobLogHandleFunc handle = print_handles[(size_t)id];
            if (!handle) {
                printf("$%c", id);
            } else {
                handle(list);
            }
            i++;
        } else {
            putchar(fmt[i]);
        }
    }
}

void logvln(const char *fmt, va_list list) {
    logv(fmt, list);
    putchar('\n');
}

void logs(const char *fmt, ...) {
    va_list list;
    va_start(list, fmt);
    logv(fmt, list);
    va_end(list);
}


void logln(const char *fmt, ...) {
    va_list list;
    va_start(list, fmt);
    logvln(fmt, list);
    va_end(list);
}
