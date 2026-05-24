#pragma once
#include <stdbool.h>
#include <stddef.h>

bool get_cmd_path(const char *path, const char *cmd, char *buffer,
                  size_t buffer_size);
