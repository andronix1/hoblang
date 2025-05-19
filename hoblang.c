#include "core/attributes.h"
#include "core/file_content.h"
#include "core/log.h"
#include "core/slice.h"
#include "lexer/token.h"
#include "sema/api.h"
#include "sema/module/api.h"
#include "sema/module/api/type.h"
#include "sema/project.h"
#include "llvm/module/api.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

static void hob_log_cstr(va_list list) { printf("%s", va_arg(list, char*)); }
static void UNUSED hob_log_int(va_list list UNUSED) { printf("%d", va_arg(list, int)); }
static void hob_log_size(va_list list) { printf("%lu", va_arg(list, size_t)); }
static void UNUSED hob_log_hex(va_list list UNUSED) { printf("%lx", va_arg(list, long)); }
static void log_errno(va_list list UNUSED) { printf("%s", strerror(errno)); }
static void hob_log_slice(va_list list) {
    Slice arg = va_arg(list, Slice);
    fwrite(arg.value, 1, arg.length, stdout);
}

int main(int argc, char **argv) {
    log_register('s', hob_log_cstr);
    log_register('S', hob_log_slice);
    log_register('l', hob_log_size);
    log_register('L', file_pos_print);
    log_register('V', file_in_lines_view_print);
    log_register('T', token_print);
    log_register('t', sema_type_print);
    log_register('E', log_errno);
    if (argc != 2) {
        logln("usage: $s <path>", argv[0]);
        return 1;
    }

    SemaProject *project = sema_project_new(argv[1]);
    if (!project) return 1;
    if (!sema_project_analyze(project)) return 1;

    SemaModule **modules = sema_project_modules(project);
    LlvmModule *llvm = llvm_module_new();
    logln("emitting $l modules...", vec_len(modules));
    for (size_t i = 0; i < vec_len(modules); i++) llvm_module_read(llvm, modules[i]);
    for (size_t i = 0; i < vec_len(modules); i++) llvm_module_emit(llvm, modules[i]);
    llvm_module_write_ir(llvm, "test.ll");
    if (!llvm_module_write_obj(llvm, "test.o")) {
        logln("failed to write module");
        return 1;
    }

    sema_project_free(project);
    llvm_module_free(llvm);
    return 0;
}
