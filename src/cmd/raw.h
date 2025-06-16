#pragma once

#include "core/mempool.h"
#include "core/slice.h"

typedef struct {
    int count;
    char **values;
} RawArgs;

static inline RawArgs raw_args_new(int count, char **values) {
    RawArgs args = {
        .count = count,
        .values = values,
    };
    return args;
}

typedef enum {
    RAW_FLAG_EMPTY,
    RAW_FLAG_VALUE,
    RAW_FLAG_LIST,
} RawFlagKind;

typedef struct {
    RawFlagKind kind;

    union {
        Slice value;
        Slice *list;
    };
} RawFlag;

static inline RawFlag raw_flag_new_value(Slice value) {
    RawFlag flag = { .kind = RAW_FLAG_VALUE, .value = value };
    return flag;
}

static inline RawFlag raw_flag_new_list(Slice *list) {
    RawFlag flag = { .kind = RAW_FLAG_LIST, .list = list };
    return flag;
}

static inline RawFlag raw_flag_new_empty() {
    RawFlag flag = { .kind = RAW_FLAG_EMPTY };
    return flag;
}

typedef struct {
    char *exe;
    char *command;
    RawFlag *flags_map;
    char **pos_args;

    size_t pos;
} RawCmd;

char *raw_cmd_take_pos(RawCmd *cmd);
RawCmd *raw_cmd_parse(Mempool *mempool, RawArgs args);
