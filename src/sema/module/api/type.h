#pragma once

#include <stdarg.h>
#include <stdbool.h>

typedef struct SemaType SemaType;

bool sema_type_is_uint(const SemaType *type);
bool sema_type_is_any_int(const SemaType *type);
bool sema_type_is_bool(const SemaType *type);
bool sema_type_eq(const SemaType *a, const SemaType *b);
bool sema_type_is_void(const SemaType *type);

void sema_type_print(va_list list);
