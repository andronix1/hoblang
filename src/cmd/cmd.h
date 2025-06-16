#pragma once

#include "cmd/raw.h"
#include "core/path.h"

typedef struct {
     Path entry;
     Slice *additional_lib_dirs;
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

static inline void cmd_setup_build_exe(Cmd *cmd, char *output, CmdSources sources, bool run) {
    cmd->kind = CMD_BUILD;
    cmd->build.kind = CMD_BUILD_EXE;
    cmd->build.output = output;
    cmd->build.sources = sources;
    cmd->build.exe.run = run;
}

static inline void cmd_setup_build_obj(Cmd *cmd, char *output, CmdSources sources) {
    cmd->kind = CMD_BUILD;
    cmd->build.kind = CMD_BUILD_OBJ;
    cmd->build.output = output;
    cmd->build.sources = sources;
}

static inline void cmd_setup_emit(Cmd *cmd, CmdEmitKind kind, char *output, CmdSources sources) {
    cmd->kind = CMD_EMIT;
    cmd->emit.kind = kind;
    cmd->emit.output = output;
    cmd->emit.sources = sources;
}

static inline void cmd_setup_help(Cmd *cmd) {
    cmd->kind = CMD_HELP;
}

Cmd *cmd_parse(Mempool *mempool, RawCmd *raw);
