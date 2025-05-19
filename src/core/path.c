#include "path.h"
#include "core/mempool.h"
#include "core/vec.h"
#include <assert.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Path path_realpath_in(Mempool *mempool, Path path) {
    char buf[PATH_MAX];
    char *real_path = realpath(path, buf);
    assert(real_path);
    size_t len = strlen(real_path);
    Path output = vec_new_in(mempool, char);
    for (size_t i = 0; i <= len; i++) {
        vec_push(output, path[i]);
    }
    return output;
}

Path path_dirname_in(Mempool *mempool, Path path) {
    size_t len = strlen(path);
    size_t last_slash = len;
    for (size_t i = 0; i < len; i++) {
        if (path[i] == '/') {
            last_slash = i;
        }
    }
    Path output = vec_new_in(mempool, char);
    for (size_t i = 0; i < last_slash; i++) {
        vec_push(output, path[i]);
    }
    vec_push(output, 0);
    return output;
}

Path path_join_in(Mempool *mempool, Path left, Path right) {
    char path[PATH_MAX];
    size_t idx = 0;

    size_t left_len = strlen(left);
    size_t right_len = strlen(right);

    for (size_t i = 0; i < left_len; i++) {
        path[idx++] = left[i];
        assert(idx < PATH_MAX);
    }

    bool ends_with_slash = left[left_len - 1] == '/';
    bool starts_with_slash = right[0] == '/';

    if (!ends_with_slash && !starts_with_slash) {
        path[idx++] = '/';
        assert(idx < PATH_MAX);
    }

    if (starts_with_slash) {
        right++;
        right_len--;
    }

    for (size_t i = 0; i < right_len; i++) {
        path[idx++] = right[i];
        assert(idx < PATH_MAX);
    }

    path[idx++] = 0;

    return path_realpath_in(mempool, path);
}
