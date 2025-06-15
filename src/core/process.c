#include "process.h"
#include "core/assert.h"
#include "core/log.h"
#include "core/vec.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

bool process_run(char *file, char **args, int *status) {
    char **real_args = vec_new(char*);
    vec_push(real_args, file);
    vec_extend(real_args, args);
    vec_push(real_args, NULL);

    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        if (execv(file, real_args) < 0) {
            logln("error: failed to run child process: $E");
            return false;
        }
        UNREACHABLE;
    }

    waitpid(pid, status, 0);
    vec_free(real_args);
    return true;
}
