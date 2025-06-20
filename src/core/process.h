#pragma once

#include <stdbool.h>

void process_run_attached(char *file, char **args);
bool process_run(char *file, char **args, int *status);
