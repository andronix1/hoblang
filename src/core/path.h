#pragma once

#include "core/mempool.h"
#include <linux/limits.h>

typedef char *Path;

Path path_realpath_in(Mempool *mempool, Path path);
Path path_dirname_in(Mempool *mempool, Path path);
Path path_join_in(Mempool *mempool, Path left, Path right);
