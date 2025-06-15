#include "cmd.h"
#include "core/log.h"
#include "core/mempool.h"
#include "core/null.h"
#include <string.h>

static inline char *args_take(Args *args) {
    if (args->count <= 0) return NULL;
    args->count--;
    char *result = args->values[0];
    args->values = &args->values[1];
    return result;
}

static inline char *args_take_req(Args *args, const char *what) {
    char *result = args_take(args);
    if (!result) {
        logln("error: missing required arg: $s", what);
    }
    return result;
}

static inline void cmd_setup_build(Cmd *cmd, CmdBuildKind kind, char *output, char *entry) {
    cmd->kind = CMD_BUILD;
    cmd->build.kind = kind;
    cmd->build.output = output;
    cmd->build.sources.entry = entry;
}

static inline void cmd_setup_emit(Cmd *cmd, CmdEmitKind kind, char *output, char *entry) {
    cmd->kind = CMD_EMIT;
    cmd->emit.kind = kind;
    cmd->emit.output = output;
    cmd->emit.sources.entry = entry;
}

static inline void cmd_setup_help(Cmd *cmd) {
    cmd->kind = CMD_HELP;
}

Cmd *cmd_parse(Mempool *mempool, Args args) {
    Cmd *cmd = mempool_alloc(mempool, Cmd);
    memset(cmd, 0, sizeof(Cmd));
    cmd->executable = NOT_NULL(args_take_req(&args, "executable"));
    
    char *name = args_take(&args);
    if (!name || !strcmp(name, "help")) {
        cmd_setup_help(cmd);
        return cmd;
    }

    if (!strcmp(name, "build-obj")) {
        char *input = NOT_NULL(args_take_req(&args, "entry file path"));
        char *output = NOT_NULL(args_take_req(&args, "output path"));
        cmd_setup_build(cmd, CMD_BUILD_OBJ, output, input);
        return cmd;
    }

    if (!strcmp(name, "build-exe")) {
        char *input = NOT_NULL(args_take_req(&args, "entry file path"));
        char *output = NOT_NULL(args_take_req(&args, "output path"));
        cmd_setup_build(cmd, CMD_BUILD_EXE, output, input);
        return cmd;
    }

    if (!strcmp(name, "emit-hir")) {
        char *input = NOT_NULL(args_take_req(&args, "input file path"));
        char *output = NOT_NULL(args_take_req(&args, "output path"));
        cmd_setup_emit(cmd, CMD_EMIT_HIR, output, input);
        return cmd;
    }

    if (!strcmp(name, "emit-llvm")) {
        char *input = NOT_NULL(args_take_req(&args, "input file path"));
        char *output = NOT_NULL(args_take_req(&args, "output path"));
        cmd_setup_emit(cmd, CMD_EMIT_IR, output, input);
        return cmd;
    }
    
    logln("unknown command `$s`", name);
    return NULL;
}
