#include "raw.h"
#include "core/vec.h"
#include <stdbool.h>
#include <string.h>

static inline char *args_take(RawArgs *args) {
    if (args->count <= 0) return NULL;
    args->count--;
    char *result = args->values[0];
    args->values = &args->values[1];
    return result;
}

static inline bool is_flag(char *arg) {
    size_t len = strlen(arg);
    return len >= 2 && arg[0] == '-' && arg[1] == '-';
}

char *raw_cmd_take_pos(RawCmd *cmd) {
    if (vec_len(cmd->pos_args) <= cmd->pos) return NULL;
    return cmd->pos_args[cmd->pos++];
}

RawCmd *raw_cmd_parse(Mempool *mempool, RawArgs args) {
    RawCmd *cmd = mempool_alloc(mempool, RawCmd);
    cmd->exe = args_take(&args);
    cmd->command = args_take(&args);
    cmd->flags_map = keymap_new_in(mempool, RawFlag);
    cmd->pos_args = vec_new_in(mempool, char*);
    cmd->pos = 0;

    char *arg = args_take(&args);
    while (arg) {
        if (is_flag(arg)) {
            SplitSlice split = slice_lsplit(subslice_from(slice_from_cstr(arg), 2), '=');
            if (split.found) {
                SplitSlice split_list = slice_lsplit(split.right, ',');
                if (!split_list.found) {
                    keymap_insert(cmd->flags_map, split.left, raw_flag_new_value(split_list.left));
                } else {
                    Slice *list = vec_new_in(mempool, Slice);
                    while (split_list.found) {
                        vec_push(list, split_list.left);
                        split_list = slice_lsplit(split_list.right, ',');
                    }
                    vec_push(list, split_list.right);
                    keymap_insert(cmd->flags_map, split.left, raw_flag_new_list(list));
                }
                
            } else {
                keymap_insert(cmd->flags_map, split.left, raw_flag_new_empty());
            }
        } else {
            vec_push(cmd->pos_args, arg);
        }
        arg = args_take(&args);
    }

    return cmd;
}
