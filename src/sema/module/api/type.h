#pragma once

#include <stdarg.h>
#include <stdbool.h>

typedef struct SemaType SemaType;

bool sema_type_eq(const SemaType *a, const SemaType *b);

void sema_type_print(va_list list);
