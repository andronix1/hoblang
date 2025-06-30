#include "const.h"
#include "sema/module/type.h"

SemaConst *sema_const_new_undefined(Mempool *mempool, SemaType *type)
    MEMPOOL_CONSTRUCT(SemaConst, 
        out->kind = SEMA_CONST_UNDEFINED;
        out->type = type;
    )

SemaConst *sema_const_new_float(Mempool *mempool, SemaType *type, long double value)
    MEMPOOL_CONSTRUCT(SemaConst, 
        out->kind = SEMA_CONST_FLOAT;
        out->type = type;
        out->float_value = value;
    )
    
SemaConst *sema_const_new_integer(Mempool *mempool, SemaType *type, uint64_t integer)
    MEMPOOL_CONSTRUCT(SemaConst, 
        out->kind = SEMA_CONST_INT;
        out->type = type;
        out->integer = integer;
    )

SemaConst *sema_const_new_bool(Mempool *mempool, bool value)
    MEMPOOL_CONSTRUCT(SemaConst, 
        out->kind = SEMA_CONST_INT;
        out->type = sema_type_new_bool(mempool);
        out->boolean = value;
    )

SemaConst *sema_const_new_struct(Mempool *mempool, SemaType *type, SemaConst **fields)
    MEMPOOL_CONSTRUCT(SemaConst, 
        out->kind = SEMA_CONST_STRUCT;
        out->type = type;
        out->struct_fields = fields;
    )

SemaConst *sema_const_new_func_gen(Mempool *mempool, SemaType *type, HirDeclId decl)
    MEMPOOL_CONSTRUCT(SemaConst, 
        out->kind = SEMA_CONST_FUNC;
        out->type = type;
        out->func_decl = decl;
    )

SemaConst *sema_const_new_gen_func(Mempool *mempool, SemaType *type, HirGenScopeId scope, HirGenFuncId func, SemaType **params)
    MEMPOOL_CONSTRUCT(SemaConst, 
        out->kind = SEMA_CONST_GEN_FUNC;
        out->type = type;
        out->gen_func.scope = scope;
        out->gen_func.func = func;
        out->gen_func.params = params;
    )

SemaConst *sema_const_new_func(Mempool *mempool, SemaType *type, HirDeclId decl)
    MEMPOOL_CONSTRUCT(SemaConst, 
        out->kind = SEMA_CONST_FUNC;
        out->type = type;
        out->func_decl = decl;
    )
