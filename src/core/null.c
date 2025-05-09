#include "null.h"
#include <stddef.h>

bool equals_nullable(const void *a, const void *b, EqFunc eq) {
    if (a == NULL || a == NULL) {
        if (a != b) {
            return false;
        }
    } else if (!eq(a, b)) {
        return false;
    }
    return true;
}
