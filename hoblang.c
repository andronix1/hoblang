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

static SemaProject *cmd_sema_project(CmdSources *sources, Ir *ir) {
    SemaProject *project = sema_project_new(ir, "../libs");
    sema_project_add_module(project, NULL, sources->entry);
    return project;
}

static void complete_ir(Ir *ir) {
    IrTypeCrossReference *crs = ir_type_check_tree(ir);
    assert(vec_len(crs) == 0);
    ir_fill_stmts(ir);
    ir_check_cosistency(ir);
}

static bool cmd_build(CmdBuild *build) {
    Ir *ir = ir_new();
    SemaProject *project = cmd_sema_project(&build->sources, ir);
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

static bool cmd_emit(CmdEmit *emit) {
    Ir *ir = ir_new();
    SemaProject *project = cmd_sema_project(&emit->sources, ir);
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

static bool cmd_help() {
    logs(
        "hoblang emit-hir <entry> <output>  - emit hoblang IR\n"
        "hoblang emit-llvm <entry> <output> - emit LLVM IR\n"
        "hoblang build-exe <entry> <output> - build executable\n"
        "hoblang build-obj <entry> <output> - build object file\n"
    );
    return true;
}

static bool cmd_version() {
    logln("hoblang :P");
    return true;
}

static bool cmd_exec(Cmd *cmd) {
    switch (cmd->kind) {
        case CMD_VERSION: return cmd_version();
        case CMD_HELP: return cmd_help();
        case CMD_BUILD: return cmd_build(&cmd->build);
        case CMD_EMIT: return cmd_emit(&cmd->emit);
    }
    UNREACHABLE;
}

int main(int argc, char **argv) {
    setup_log();

    Mempool *mempool = mempool_new(256);

    Cmd *cmd = cmd_parse(mempool, args_new(argc, argv));
    if (!cmd) {
        cmd_help();
        mempool_free(mempool);
        return 1;
    }
    bool result = cmd_exec(cmd);

    mempool_free(mempool);
    return result ? 0 : 1;
}
