#pragma once

#include "sema/module/api/const.h"
#include "sema/module/api/type.h"
#include <stdbool.h>

typedef enum {
    SEMA_STD_UNLOADED,
    SEMA_STD_FAILED,
    SEMA_STD_LOADED,
} SemaStdState;

typedef struct {
    SemaStdState state;
    SemaType *usize;
    struct {
        SemaType *type;
    } string;
} SemaStd;

static inline SemaStd sema_std_module_new() {
    SemaStd str = { .state = SEMA_STD_UNLOADED };
    return str;
}

bool sema_module_std_load(SemaModule *module, Slice at);
SemaType *sema_module_std_string(SemaModule *module, Slice at);
SemaType *sema_module_std_usize(SemaModule *module, Slice at);
SemaConst *sema_module_std_new_string(SemaModule *module, Slice at, SemaConst *pointer, SemaConst *len);

