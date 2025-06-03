#include "type_tree.h"
#include "ir/ir.h"
#include "ir/type/type.h"
#include <assert.h>
#include <stddef.h>

static IrTypeDeps *ir_type_build_deps(const Ir *ir) {
    size_t count = vec_len(ir->types);

    IrTypeDeps *deps = vec_new_in(ir->mempool, IrTypeDeps);
    vec_resize(deps, count);

    for (size_t i = 0; i < count; i++) {
        size_t id = ir_type_record_resolve_simple(ir, i);
        deps[i].deps = ir_type_depends_on(ir->mempool, &ir->types[id].simple);
    }

    return deps;
}

typedef struct {
    bool visited;
    bool visited_now;
} IrTypeDepInfo;

static IrTypeDepInfo *ir_type_build_dep_infos(const Ir *ir) {
    size_t count = vec_len(ir->types);

    IrTypeDepInfo *infos = vec_new_in(ir->mempool, IrTypeDepInfo);
    vec_resize(infos, count);
    for (size_t i = 0; i < count; i++) {
        infos[i].visited = false;
        infos[i].visited_now = false;
    }

    return infos;
}

static void ir_type_append_cross_references_for(
    IrTypeId id, IrTypeId from,
    IrTypeDeps *deps, IrTypeDepInfo *infos,
    IrTypeCrossReference **crs
) {
    assert(vec_len(deps) == vec_len(infos));

    IrTypeDepInfo *info = &infos[id];
    if (info->visited_now) {
        IrTypeCrossReference cr = {
            .id = id,
            .what = from,
        };
        vec_push(*crs, cr);
    }
    if (info->visited) {
        return;
    }
    info->visited = info->visited_now = true;

    IrTypeDeps *dep = &deps[id];
    for (size_t i = 0; i < vec_len(dep->deps); i++) {
        ir_type_append_cross_references_for(dep->deps[i], dep->deps[i], deps, infos, crs);
    }

    info->visited_now = false;
}

static IrTypeCrossReference *ir_type_get_cross_references(
    Mempool *mempool,
    IrTypeDeps *deps,
    IrTypeDepInfo *infos
) {
    size_t count = vec_len(deps);
    assert(count == vec_len(infos));
    IrTypeCrossReference *crs = vec_new_in(mempool, IrTypeCrossReference);
    for (size_t i = 0; i < count; i++) {
        ir_type_append_cross_references_for(i, -1, deps, infos, &crs);
    }
    return crs;
}

IrTypeCrossReference *ir_type_check_tree(Ir *ir) {
    IrTypeDeps *deps = ir_type_build_deps(ir);
    IrTypeDepInfo *infos = ir_type_build_dep_infos(ir);
    IrTypeCrossReference *crs = ir_type_get_cross_references(ir->mempool, deps, infos);
    return crs;
}

