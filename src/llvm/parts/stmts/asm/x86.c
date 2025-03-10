#include "llvm/private.h"
#include "llvm/parts/type.h"
#include "llvm/parts/expr.h"
#include "sema/value/api.h"
#include "sema/value/private.h"
#include "core/vec.h"
#include "ast/private/stmts/asm.h"
#include "ast/private/expr.h"

void llvm_stmt_asm(LlvmBackend *llvm, AstInlineAsm *inline_asm) {
    #define APPEND_ASM_PTR(ptr, len) do { assembly = vec_append_raw(assembly, ptr, len); } while (0)
    #define APPEND_ASM(fmt, ...) APPEND_ASM_PTR(buf, sprintf(buf, fmt, ##__VA_ARGS__))
    #define APPEND_CONSTR_RAW(ptr, len) do { \
        constraint = vec_append_raw(constraint, ptr, len); \
    } while (0)
    #define APPEND_CONSTR_PTR(ptr, len) do { \
        if (args_count > 1) { \
            APPEND_CONSTR_RAW(",", 1); \
        } \
        APPEND_CONSTR_RAW(ptr, len); \
    } while (0)
    #define APPEND_CONSTR(fmt, ...) do { \
            APPEND_CONSTR_PTR(buf, sprintf(buf, fmt, ##__VA_ARGS__)); \
        } while (0)

    char *assembly = vec_new(char);
    char *constraint = vec_new(char);
    size_t args_count = 0;
    bool has_side_effect = true;
    char buf[32] = {0};
    LLVMValueRef *values = vec_new(LLVMValueRef);
    LLVMTypeRef *types = vec_new(LLVMTypeRef);

    for (size_t i = 0; i < vec_len(inline_asm->mnems); i++) {
        AstAsmMnemonic *mnemonic = &inline_asm->mnems[i];
        APPEND_ASM_PTR(mnemonic->name.str, mnemonic->name.len);
        APPEND_ASM(" ");
        for (size_t j = 0; j < vec_len(mnemonic->args); j++) {
            if (j != 0) {
                APPEND_ASM(",");
            }
            AstAsmArg *arg = &mnemonic->args[j];
            switch (arg->type) {
                case AST_ASM_ARG_REGISTER:
                    APPEND_ASM_PTR(arg->reg.str, arg->reg.len);
                    break;

                case AST_ASM_ARG_EXPR: {
                    LLVMValueRef value = llvm_expr(llvm, arg->expr, true);
                    values = vec_push(values, &value);
                    LLVMTypeRef type = llvm_resolve_type(sema_value_typeof(arg->expr->value));
                    types = vec_push(types, &type);
                    APPEND_ASM("$%lu", args_count);
                    args_count++;
                    APPEND_CONSTR("rim");
                    break;
                }
                case AST_ASM_ARG_ADDRESS: {
                    LLVMValueRef value = LLVMBuildPtrToInt(
                        llvm_builder(llvm),
                        llvm_expr(llvm, arg->expr, false),
                        LLVMInt64Type(),
                        ""
                    );
                    values = vec_push(values, &value);
                    LLVMTypeRef type = LLVMInt64Type();
                    types = vec_push(types, &type);
                    APPEND_ASM("$%ld", args_count);
                    args_count++;
                    APPEND_CONSTR("p");
                    break;
                }
            }
        }
        APPEND_ASM("\n");
    }

    for (size_t i = 0; i < vec_len(inline_asm->clobbers); i++) {
        APPEND_CONSTR("~{");
        args_count++;
        Slice *clobber = &inline_asm->clobbers[i];
        APPEND_CONSTR_RAW(clobber->str, clobber->len);
        APPEND_CONSTR_RAW("}", 1);
    }

    LLVMValueRef call = LLVMGetInlineAsm(
        LLVMVoidType(),
        assembly,
        vec_len(assembly),
        constraint,
        vec_len(constraint),
        has_side_effect,
        false, // required_aligned_stack
        LLVMInlineAsmDialectIntel,
        false
    );
    LLVMSetVolatile(call, inline_asm->is_volatile);
    LLVMBuildCall2(llvm_builder(llvm), LLVMFunctionType(LLVMVoidType(), types, vec_len(types), false), call, values, vec_len(values), "");
}
