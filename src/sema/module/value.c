#include "value.h"
#include "core/mempool.h"
#include <stdio.h>

void sema_value_print(va_list list) {
    SemaValue *value = va_arg(list, SemaValue*);
    switch (value->kind) {
        case SEMA_VALUE_TYPE: printf("type"); break;
    }
}

SemaValue *sema_value_new_type(Mempool *mempool, SemaType *type)
    MEMPOOL_CONSTRUCT(SemaValue,
        out->kind = SEMA_VALUE_TYPE;
        out->type = type
    );
