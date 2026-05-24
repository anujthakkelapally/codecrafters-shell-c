#include "paths.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

bool get_cmd_path(const char *path, const char *cmd, char *buffer,
                  size_t buffer_size) {
    if (path == NULL || cmd == NULL || buffer == NULL) {
        return false;
    }
    char *path_copy = strdup(path);
    if (path_copy == NULL) {
        return false;
    }
    char *saveptr;
    char *dir = strtok_r(path_copy, ":", &saveptr);
    while (dir != NULL) {
        snprintf(buffer, buffer_size, "%s/%s", dir, cmd);
        if (access(buffer, X_OK) == 0) {
            free(path_copy);
            return true;
        }
        dir = strtok_r(NULL, ":", &saveptr);
    }
    free(path_copy);
    return false;
}
