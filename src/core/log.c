#include "log.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    size_t size;
    HobLogHandleFunc func;
} HobLogHandle;

static HobLogHandle print_handles[1 << (sizeof(char) * 8)] = {
    { .size = 0, .func = NULL },
};

void log_register(char id, size_t size, HobLogHandleFunc func) {
    assert(print_handles[id].func == NULL);
    print_handles[id].size = size;
    print_handles[id].func = func;
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
            HobLogHandle handle = print_handles[id];
            if (!handle.func) {
                printf("$%c", id);
            } else {
                handle.func(list);
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

void logln(const char *fmt, ...) {
    va_list list;
    va_start(list, fmt);
    logvln(fmt, list);
    va_end(list);
}
