#include "cmd/cmd.h"
#include "core/assert.h"
#include "core/log.h"
#include "core/mempool.h"
#include "core/process.h"
#include "core/vec.h"
#include "ir/api/ir.h"
#include "ir/dump/dump.h"
#include "ir/stages/checks.h"
#include "ir/stages/stmts.h"
#include "ir/stages/type_tree.h"
#include "print.h"
#include "sema/api/project.h"
#include "llvm/module/api.h"

static SemaProject *cmd_sema_project(Mempool *mempool, CmdSources *sources, Ir *ir) {
    Path *lib_dirs = vec_create_in(mempool, (char*)"../libs", "./libs");
    for (size_t i = 0; i < vec_len(sources->additional_lib_dirs); i++) {
        vec_push(lib_dirs, mempool_slice_to_cstr(mempool, sources->additional_lib_dirs[i]));
    }
    SemaProject *project = sema_project_new(ir, lib_dirs);
    sema_project_add_module(project, NULL, sources->entry, false);
    return project;
}

static void complete_ir(Ir *ir) {
    IrTypeCrossReference *crs = ir_type_check_tree(ir);
    assert(vec_len(crs) == 0);
    ir_fill_stmts(ir);
    ir_check_cosistency(ir);
}

static bool cmd_build(Mempool *mempool, CmdBuild *build) {
    Ir *ir = ir_new();
    SemaProject *project = cmd_sema_project(mempool, &build->sources, ir);
    if (!project) {
        ir_free(ir);
        return false;
    }
    sema_project_emit(project);

    if (sema_project_failed(project)) {
        sema_project_free(project);
        ir_free(ir);
        return false;
    }

    complete_ir(ir);

    LlvmModule *llvm = llvm_module_new();
    llvm_module_emit(llvm, ir);

    char *temp_obj_path = "/tmp/hoblang-obj.o";

    if (!llvm_module_write_obj(llvm, build->kind == CMD_BUILD_OBJ ? build->output : temp_obj_path)) {
        sema_project_free(project);
        llvm_module_free(llvm);
        ir_free(ir);
        return false;
    }

    bool result = true;

    switch (build->kind) {
        case CMD_BUILD_OBJ: break;
        case CMD_BUILD_EXE: {
            int status;
            char **args = vec_create(temp_obj_path, "-o", build->output);
            result = process_run("/usr/bin/gcc", args, &status);
            vec_free(args);
            if (status) {
                logln("linker failed with status $l", status);
                break;
            }

            if (build->exe.run) {
                int status;
                char **args = vec_new(char*);
                result = process_run(build->output, args, &status);
                if (result && status) {
                    logln("program failed with status $l", status);
                }
                vec_free(args);
            }
            break;
        }
    }

    sema_project_free(project);
    llvm_module_free(llvm);
    ir_free(ir);

    return result;
}

static bool cmd_emit(Mempool *mempool, CmdEmit *emit) {
    Ir *ir = ir_new();
    SemaProject *project = cmd_sema_project(mempool, &emit->sources, ir);
    if (!project) {
        ir_free(ir);
        return false;
    }
    sema_project_emit(project);

    if (sema_project_failed(project)) {
        sema_project_free(project);
        ir_free(ir);
        return false;
    }
    complete_ir(ir);

    switch(emit->kind) {
        case CMD_EMIT_HIR: {
            bool result = ir_dump(ir, emit->output);
            sema_project_free(project);
            ir_free(ir);
            return result;
        }
        case CMD_EMIT_IR: {
            LlvmModule *llvm = llvm_module_new();
            if (!llvm) {
                sema_project_free(project);
                ir_free(ir);
                return false;
            }
            llvm_module_emit(llvm, ir);

            bool result = llvm_module_write_ir(llvm, emit->output);
            llvm_module_free(llvm);
            sema_project_free(project);
            ir_free(ir);
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
    FLAG_VALUE("libDirs", "[lib1,lib2,...]", "add library search directories");
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
    });
    HELP_COMMAND("build-obj", "<entry> <output>", "emit executable", {
        cmd_help_sources();
        FLAG_EMPTY("run", "run executable after successful build");
    });
    return true;
}

static bool cmd_version() {
    logln("hoblang :P");
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
    setup_log();

    Mempool *mempool = mempool_new(256);

    Cmd *cmd = cmd_parse(mempool, raw_cmd_parse(mempool, raw_args_new(argc, argv)));
    if (!cmd) {
        cmd_help(cmd->executable);
        mempool_free(mempool);
        return 1;
    }
    bool result = cmd_exec(mempool, cmd);

    mempool_free(mempool);
    return result ? 0 : 1;
}
