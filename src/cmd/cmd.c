#include "cmd.h"
#include "core/assert.h"
#include "core/log.h"
#include "core/mempool.h"
#include "core/null.h"
#include <string.h>

static inline char *raw_cmd_take_pos_req(RawCmd *cmd, char *name) {
    char *result = raw_cmd_take_pos(cmd);
    if (!result) {
        logln("error: missing required positional argument: $s", name);
    }
    return result;
}

static inline bool raw_cmd_check_flag(RawCmd *cmd, const char *name, bool *output) {
    RawFlag *flag = keymap_get(cmd->flags_map, slice_from_cstr(name));
    if (!flag) {
        *output = false;
        return true;
    } else {
        if (flag->kind != RAW_FLAG_EMPTY) {
            logln("error: flag `$s` must not have value attached", name);
            return false;
        }
        *output = true;
        return true;
    }
}

static inline bool raw_cmd_resolve_opt_value(RawCmd *cmd, const char *name, Slice **output) {
    RawFlag *flag = keymap_get(cmd->flags_map, slice_from_cstr(name));
    if (!flag) {
        *output = NULL;
        return true;
    } else {
        switch (flag->kind) {
            case RAW_FLAG_EMPTY:
            case RAW_FLAG_LIST:
                logln("error: flag `$s` must be a value", name);
                return false;
            case RAW_FLAG_VALUE:
                *output = &flag->value;
                return true;
        }
        UNREACHABLE;
    }
}

static inline Slice *raw_cmd_resolve_opt_list(Mempool *mempool, RawCmd *cmd, const char *name) {
    RawFlag *flag = keymap_get(cmd->flags_map, slice_from_cstr(name));
    if (!flag) {
        return vec_new_in(mempool, Slice);
    } else {
        switch (flag->kind) {
            case RAW_FLAG_EMPTY:
                logln("error: flag `$s` must be a list", name);
                return NULL;
            case RAW_FLAG_VALUE:
                return vec_create_in(mempool, flag->value);
            case RAW_FLAG_LIST:
                return flag->list;
        }
        UNREACHABLE;
    }
}

static inline bool cmd_sources_parse(Mempool *mempool, RawCmd *raw, CmdSources *sources) {
    sources->entry = NOT_NULL(raw_cmd_take_pos_req(raw, "entry file path"));
    sources->additional_lib_dirs = NOT_NULL(raw_cmd_resolve_opt_list(mempool, raw, "libDirs"));
    return true;
}

Cmd *cmd_parse(Mempool *mempool, RawCmd *raw) {
    assert(raw->exe);

    Cmd *cmd = mempool_alloc(mempool, Cmd);
    memset(cmd, 0, sizeof(Cmd));
    cmd->executable = raw->exe;
    
    if (!raw->command) {
        cmd_setup_help(cmd);
        return cmd;
    }

    if (!strcmp(raw->command, "emit-llvm")) {
        CmdSources sources;
        NOT_NULL(cmd_sources_parse(mempool, raw, &sources));
        char *output = NOT_NULL(raw_cmd_take_pos_req(raw, "output path"));
        cmd_setup_emit(cmd, CMD_EMIT_IR, output, sources);
        return cmd;
    }

    if (!strcmp(raw->command, "emit-hir")) {
        CmdSources sources;
        NOT_NULL(cmd_sources_parse(mempool, raw, &sources));
        char *output = NOT_NULL(raw_cmd_take_pos_req(raw, "output path"));
        cmd_setup_emit(cmd, CMD_EMIT_HIR, output, sources);
        return cmd;
    }

    if (!strcmp(raw->command, "build-obj")) {
        CmdSources sources;
        NOT_NULL(cmd_sources_parse(mempool, raw, &sources));
        char *output = NOT_NULL(raw_cmd_take_pos_req(raw, "output path"));
        cmd_setup_build_obj(cmd, output, sources);
        return cmd;
    }

    if (!strcmp(raw->command, "build-exe")) {
        CmdSources sources;
        NOT_NULL(cmd_sources_parse(mempool, raw, &sources));
        char *output = NOT_NULL(raw_cmd_take_pos_req(raw, "output path"));
        Slice *linker_path;
        NOT_NULL(raw_cmd_resolve_opt_value(raw, "linker", &linker_path));
        Slice *linker_flags = NOT_NULL(raw_cmd_resolve_opt_list(mempool, raw, "linkerFlags"));
        bool run;
        NOT_NULL(raw_cmd_check_flag(raw, "run", &run));
        cmd_setup_build_exe(cmd, output, sources, linker_path, linker_flags, run);
        return cmd;
    }
    
    logln("unknown command `$s`", raw->command);
    return NULL;
}
