#include "global.h"

OptSlice sema_global_to_opt_slice(AstGlobal *global, Slice or) {
    return global ? opt_slice_map_or(global->alias, or) : opt_slice_new_null();
}
