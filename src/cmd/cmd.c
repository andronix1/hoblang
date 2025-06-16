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

static inline void cmd_setup_build_exe(Cmd *cmd, char *output, char *entry, bool run) {
    cmd->kind = CMD_BUILD;
    cmd->build.kind = CMD_BUILD_EXE;
    cmd->build.output = output;
    cmd->build.sources.entry = entry;
    cmd->build.exe.run = run;
}

static inline void cmd_setup_build_obj(Cmd *cmd, char *output, char *entry) {
    cmd->kind = CMD_BUILD;
    cmd->build.kind = CMD_BUILD_OBJ;
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
        cmd_setup_build_obj(cmd, output, input);
        return cmd;
    }

    if (!strcmp(name, "build-exe")) {
        char *input = NOT_NULL(args_take_req(&args, "entry file path"));
        char *output = NOT_NULL(args_take_req(&args, "output path"));
        char *run_flag = args_take(&args);
        bool run = false;
        if (run_flag) {
            if (!strcmp(run_flag, "--run")) {
                run = true;
            } else {
                logln("unexpected arg `$s`", run_flag);
            }
        }
        cmd_setup_build_exe(cmd, output, input, run);
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
