#include "cmd/cmd.h"
#include "core/assert.h"
#include "core/log.h"
#include "core/mempool.h"
#include "core/process.h"
#include "core/vec.h"
#include "hir/api/dump/dump.h"
#include "hir/api/hir.h"
#include "print.h"
#include "sema/api/project.h"
#include "llvm/module/api.h"

static SemaProject *cmd_sema_project(Mempool *mempool, CmdSources *sources, Hir *hir) {
    Path *lib_dirs = vec_create_in(mempool, (char*)"../libs", "./libs", "/opt/hob/libs");
    for (size_t i = 0; i < vec_len(sources->additional_lib_dirs); i++) {
        vec_push(lib_dirs, mempool_slice_to_cstr(mempool, sources->additional_lib_dirs[i]));
    }
    SemaProject *project = sema_project_new(hir, lib_dirs);
    sema_project_add_module(project, NULL, sources->entry, false);
    return project;
}

static void complete_hir(Hir *hir) {
    hir_postprocess(hir);
}

static bool cmd_build(Mempool *mempool, CmdBuild *build) {
    Hir *hir = hir_new();
    SemaProject *project = cmd_sema_project(mempool, &build->sources, hir);
    if (!project) {
        hir_free(hir);
        return false;
    }
    sema_project_emit(project);

    if (sema_project_failed(project)) {
        sema_project_free(project);
        hir_free(hir);
        return false;
    }

    complete_hir(hir);

    LlvmModule *llvm = llvm_module_new();
    llvm_module_emit(llvm, hir);

    char *temp_obj_path = "/tmp/hoblang-obj.o";

    if (!llvm_module_write_obj(llvm, build->kind == CMD_BUILD_OBJ ? build->output : temp_obj_path)) {
        sema_project_free(project);
        llvm_module_free(llvm);
        hir_free(hir);
        return false;
    }

    bool result = true;

    switch (build->kind) {
        case CMD_BUILD_OBJ: break;
        case CMD_BUILD_EXE: {
            int status;
            char **args = vec_create(temp_obj_path, "-o", build->output);
            for (size_t i = 0; i < vec_len(build->exe.linker.flags); i++) {
                vec_push(args, mempool_slice_to_cstr(mempool, build->exe.linker.flags[i]));
            }
            result = process_run(
                build->exe.linker.path ? mempool_slice_to_cstr(mempool, *build->exe.linker.path) : "/usr/bin/gcc",
                args,
                &status
            );
            vec_free(args);
            if (status) {
                logln("linker failed with status $l", status);
                break;
            }

            if (build->exe.run) {
                process_run_attached(build->output, vec_new(char*));
            }
            break;
        }
    }

    sema_project_free(project);
    llvm_module_free(llvm);
    hir_free(hir);

    return result;
}

static bool cmd_emit(Mempool *mempool, CmdEmit *emit) {
    Hir *hir = hir_new();
    SemaProject *project = cmd_sema_project(mempool, &emit->sources, hir);
    if (!project) {
        hir_free(hir);
        return false;
    }
    sema_project_emit(project);

    if (sema_project_failed(project)) {
        sema_project_free(project);
        hir_free(hir);
        return false;
    }

    switch(emit->kind) {
        case CMD_EMIT_HIR: {
            bool result = hir_dump(hir, emit->output);
            sema_project_free(project);
            hir_free(hir);
            return result;
        }
        case CMD_EMIT_IR: {
            complete_hir(hir);
            LlvmModule *llvm = llvm_module_new();
            if (!llvm) {
                sema_project_free(project);
                hir_free(hir);
                return false;
            }
            llvm_module_emit(llvm, hir);

            bool result = llvm_module_write_ir(llvm, emit->output);
            llvm_module_free(llvm);
            sema_project_free(project);
            hir_free(hir);
            return result;
        }
    }
    UNREACHABLE;
}

#define FLAG_EMPTY(NAME, DESCRIPTION) logln("  --" NAME " - "DESCRIPTION)
#define FLAG_VALUE(NAME, VALUE, DESCRIPTION) logln("  --" NAME "=" VALUE " - "DESCRIPTION)

#define HELP_COMMAND(NAME, POS, DESCRIPTION, ARGS) do { \
        logln("$s " NAME " "POS" - "DESCRIPTION, exe); \
        ARGS; \
        logln(""); \
    } while (0)

static inline void cmd_help_sources() {
    FLAG_VALUE("libDirs", "lib1,lib2,...", "add library search directories");
}

static bool cmd_help(char *exe) {
    HELP_COMMAND("help", "", "print help", {});
    HELP_COMMAND("emit-hir", "<entry> <output>", "emit hoblang IR", {
        cmd_help_sources();
    });
    HELP_COMMAND("emit-llvm", "<entry> <output>", "emit LLVM IR", {
        cmd_help_sources();
    });
    HELP_COMMAND("build-exe", "<entry> <output>", "emit executable", {
        cmd_help_sources();
        FLAG_EMPTY("run", "run executable after successful build");
        FLAG_VALUE("linker", "/path/to/linker", "specify linker path");
        FLAG_VALUE("linkerFlags", "flag1,flag2,...", "specify additional flags for linker");
    });
    HELP_COMMAND("build-obj", "<entry> <output>", "emit executable", {
        cmd_help_sources();
    });
    return true;
}

static bool cmd_version() {
    logln("1.0.0-preview");
    return true;
}

static bool cmd_exec(Mempool *mempool, Cmd *cmd) {
    switch (cmd->kind) {
        case CMD_VERSION: return cmd_version();
        case CMD_HELP: return cmd_help(cmd->executable);
        case CMD_BUILD: return cmd_build(mempool, &cmd->build);
        case CMD_EMIT: return cmd_emit(mempool, &cmd->emit);
    }
    UNREACHABLE;
}

int main(int argc, char **argv) {
    assert(argc > 0);
    setup_log();

    Mempool *mempool = mempool_new(256);

    Cmd *cmd = cmd_parse(mempool, raw_cmd_parse(mempool, raw_args_new(argc, argv)));
    if (!cmd) {
        cmd_help(argv[0]);
        mempool_free(mempool);
        return 1;
    }
    bool result = cmd_exec(mempool, cmd);

    mempool_free(mempool);
    return result ? 0 : 1;
}
