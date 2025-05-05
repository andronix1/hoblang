#pragma once

#include <stdarg.h>
#include <stddef.h>

typedef void (*HobLogHandleFunc)(va_list);

void log_register(char id, size_t size, HobLogHandleFunc func);
void logv(const char *fmt, va_list list);
void logvln(const char *fmt, va_list list);

void logln(const char *fmt, ...);

/*
./path/to/the/file.hob:1:10: error: this is very cool error message
    call_me_maybe()
    ^^^^^^^^^^^^^
*/
