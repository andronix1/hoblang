#pragma once

#include <stdarg.h>
#include <stddef.h>

typedef void (*HobLogHandleFunc)(va_list);

void log_register(char id, HobLogHandleFunc func);
void logv(const char *fmt, va_list list);
void logvln(const char *fmt, va_list list);

void logln(const char *fmt, ...);
