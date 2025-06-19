#pragma once

#include "hir/api/const.h"
#include "hir/api/decl.h"
#include "hir/api/extern.h"
#include "hir/api/func.h"
#include "hir/api/mut.h"
#include "hir/api/type.h"
#include "hir/api/var.h"
#include "hir/decl.h"

typedef struct Hir Hir;

Hir *hir_new();
void hir_free(Hir *hir);

HirTypeId hir_add_type(Hir *hir, HirType type);
HirTypeId hir_register_type(Hir *hir);
void hir_init_type(Hir *hir, HirTypeId id, HirTypeId type);

HirGenParamId hir_add_gen_param(Hir *hir);
HirGenTypeId hir_register_gen_type(HirGenParamId *ids);
void hir_init_gen_type(Hir *hir, HirGenTypeId id, HirTypeId type);

HirDeclId hir_add_decl(Hir *hir);
void hir_init_decl_func(Hir *hir, HirDeclId id, HirFuncId func);
void hir_init_decl_extern(Hir *hir, HirDeclId id, HirExternId type);
void hir_init_decl_var(Hir *hir, HirDeclId id, HirVarId var);
const HirDeclInfo *hir_get_decls(const Hir *hir);
HirMutability hir_get_decl_mutability(Hir *hir, HirDeclId id);

HirExternId hir_add_extern(Hir *hir, Slice name, HirExternInfo info);
HirExternRecord hir_get_extern_info(const Hir *hir, HirExternId id);

HirVarId hir_add_var(Hir *hir, HirVarInfo info);
void hir_set_var_initializer(Hir *hir, HirVarId id, HirConst initializer);
const HirVarInfo *hir_get_var_info(const Hir *hir, HirVarId id);
size_t hir_get_vars_count(const Hir *hir);

HirFuncId hir_register_fun(Hir *hir, HirTypeId type);
void hir_init_fun(Hir *hir, HirFuncId id, HirMutability *args_mut, HirFuncInfo info);
void hir_init_fun_body(Hir *hir, HirFuncId id, HirCode *code);
HirLocalId hir_fun_add_local(Hir *hir, HirFuncId id, HirFuncLocal local);
HirLoopId hir_fun_add_loop(Hir *hir, HirFuncId id);
const HirFuncInfo *hir_get_func_info(const Hir *hir, HirFuncId id);
HirLocalId hir_get_func_arg_local(const Hir *hir, HirFuncId id, size_t number);

void hir_postprocess(Hir *hir);
