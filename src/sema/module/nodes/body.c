#include "body.h"
#include "ast/body.h"
#include "ast/node.h"
#include "core/slice.h"
#include "core/vec.h"
#include "sema/module/module.h"
#include "sema/module/nodes/node.h"

bool sema_module_analyze_body(SemaModule *module, AstBody *body) {
    sema_module_push_scope(module);
    bool is_unreachable = false;
    Slice unreachable;
    bool finished = false;
    for (size_t i = 0; i < vec_len(body->nodes); i++) {
        AstNode *node = body->nodes[i];
        sema_module_read_node(module, node);
        if (is_unreachable) {
            unreachable = slice_union(unreachable, node->slice);
        }
        if (sema_module_analyze_node(module, node)) {
            finished = true;
            if (!is_unreachable && i + 1 != vec_len(body->nodes)) {
                unreachable = body->nodes[i + 1]->slice;
                is_unreachable = true;
            }
        }
    }
    if (is_unreachable) {
        sema_module_err(module, unreachable, "unreachable code");
    }
    sema_module_pop_scope(module);
    return body->sema.finished = finished;
}
