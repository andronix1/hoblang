#pragma once

#include <stdbool.h>

typedef struct SemaType SemaType;

bool sema_type_eq(const SemaType *a, const SemaType *b);
bool sema_type_full_eq(const SemaType *a, const SemaType *b);
