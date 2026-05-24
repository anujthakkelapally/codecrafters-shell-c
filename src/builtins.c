#include "builtins.h"
#include "paths.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

typedef void (*builtin_fn)(char **tokens, int count);

typedef struct {
    const char *name;
    builtin_fn fn;
} builtin_entry;

bool is_builtin(const char *cmd);

void builtin_echo(char **tokens, int count) {
    for (int i = 1; i < count; i++) {
        printf("%s", tokens[i]);
        if (i != count - 1) {
            printf(" ");
        }
    }
    printf("\n");
}

void builtin_exit(char **tokens, int count) {
    if (count > 1) {
        char *end;
        long code = strtol(tokens[1], &end, 10);
        if (*end != '\0') {
            fprintf(stderr, "exit: %s: numeric argument required\n", tokens[1]);
            exit(255);
        }
        exit((int)code);
    }
    exit(0);
}

void builtin_type(char **tokens, int count) {
    if (count == 1) {
        return;
    }
    if (is_builtin(tokens[1])) {
        printf("%s is a shell builtin\n", tokens[1]);
    } else {
        char cmd_path[BUFFER_SIZE];
        bool in_path =
            get_cmd_path(getenv("PATH"), tokens[1], cmd_path, BUFFER_SIZE);
        if (in_path) {
            printf("%s is %s\n", tokens[1], cmd_path);
        } else {
            printf("%s: not found\n", tokens[1]);
        }
    }
}

void builtin_pwd(char **tokens, int count) {
    char buffer[BUFFER_SIZE];
    char *wd = getcwd(buffer, BUFFER_SIZE);
    if (wd == NULL) {
        perror("pwd");
    } else {
        printf("%s\n", buffer);
    }
}

void builtin_cd(char **tokens, int count) {
    const char *dir;
    if (count == 1 || (strcmp(tokens[1], "~") == 0)) {
        dir = getenv("HOME");
    } else {
        dir = tokens[1];
    }
    if (dir == NULL) {
        fprintf(stderr, "cd: HOME not set\n");
        return;
    }
    if (chdir(dir) != 0) {
        fprintf(stderr, "cd: %s: No such file or directory\n", dir);
    }
}

static const builtin_entry dispatch[] = {
    {"echo", builtin_echo}, {"exit", builtin_exit}, {"type", builtin_type},
    {"pwd", builtin_pwd},   {"cd", builtin_cd},     {NULL, NULL}};

bool is_builtin(const char *cmd) {
    for (int i = 0; dispatch[i].name; i++) {
        if (strcmp(cmd, dispatch[i].name) == 0) {
            return true;
        }
    }
    return false;
}

bool dispatch_builtin(char **tokens, int count) {
    for (int i = 0; dispatch[i].name != NULL; i++) {
        if (strcmp(tokens[0], dispatch[i].name) == 0) {
            dispatch[i].fn(tokens, count);
            return true;
        }
    }
    return false;
}
