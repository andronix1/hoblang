#include "cmd/private.h"
#include "core/vec.h"

bool cmd_parse_build_exe(Cmd *output, const char **args, size_t len) {
    output->build_exe.linker.libs = vec_new(const char*);
    output->build_exe.run_args = vec_new(const char*);
    CMD_PARSE {
        const char *arg = POP_ARG();
        if (CMD_IS_FLAG) {
            CMD_FLAG_APPEND("link", output->build_exe.linker.libs);
            CMD_FLAG_STR("temp-obj", output->build_exe.temp_obj);
            CMD_FLAG_ON("run", output->build_exe.run);
            CMD_FLAG_APPEND_ALL("args", output->build_exe.run_args);
            CMD_FLAGS_END;
        } else {
            CMD_POS_ARG(0, output->build_exe.input);
            CMD_POS_ARG(1, output->build_exe.output);
            CMD_POS_ARGS_END;
        }
    }
    CMD_END(2);
    CMD_DEFAULT(output->build_exe.temp_obj, "/tmp/hobtmp.o");
    return true;
}
