#pragma once

#define min(a, b) (a > b ? b : a)
#define max(a, b) (a > b ? a : b)
#define swap(a, b) do { \
    typeof(a) __temp = a; \
    a = b; \
    b = __temp; \
} while (0)
