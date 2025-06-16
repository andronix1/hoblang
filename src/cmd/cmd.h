#pragma once

#include "core/path.h"

typedef struct {
     Path entry;
} CmdSources;

typedef enum {
    CMD_BUILD_OBJ,
    CMD_BUILD_EXE,
} CmdBuildKind;

typedef struct {
    CmdBuildKind kind;
    CmdSources sources;
    Path output;

    union {
        struct {
            bool run;
        } exe;
    };
} CmdBuild;

typedef enum {
    CMD_EMIT_HIR,
    CMD_EMIT_IR,
} CmdEmitKind;

typedef struct {
    CmdEmitKind kind;
    CmdSources sources;
    Path output;
} CmdEmit;

typedef enum {
    CMD_VERSION, CMD_HELP,
    CMD_BUILD, CMD_EMIT,
} CmdKind;

typedef struct {
    CmdKind kind;

    char *executable;

    union {
        CmdBuild build;
        CmdEmit emit;
    };
} Cmd;

typedef struct {
    int count;
    char **values;
} Args;

static inline Args args_new(int count, char **values) {
    Args args = {
        .count = count,
        .values = values,
    };
    return args;
}

Cmd *cmd_parse(Mempool *mempool, Args args);
