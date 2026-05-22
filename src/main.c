#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define MAX_TOKENS 64

int read_line(char *buffer, size_t size, FILE *stream) {
    if (fgets(buffer, size, stream) == NULL) {
        return 0;
    }
    char *ptr = strchr(buffer, '\n');
    if (ptr != NULL) {
        *ptr = '\0';
    }
    return 1;
}

int tokenize(char *buffer, char **tokens, int max_tokens) {
    if (buffer == NULL)
        return 0;
    int count = 0;
    char *saveptr;
    char *token = strtok_r(buffer, " \t\n", &saveptr);
    while (token != NULL && count < max_tokens - 1) {
        tokens[count++] = token;
        token = strtok_r(NULL, " \t\n", &saveptr);
    }
    tokens[count] = NULL;
    return count;
}

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
    getcwd(buffer, BUFFER_SIZE);
    printf("%s\n", buffer);
}

void builtin_cd(char **tokens, int count) {
    const char *dir = (count > 1)? tokens[1]: getenv("HOME");
    if (dir == NULL) {
        fprintf(stderr, "cd: HOME not set\n");
        return;
    }
    if (chdir(dir) != 0) {
        fprintf(stderr, "cd: %s: No such file or directory\n", tokens[1]);
    }
}

static const builtin_entry dispatch[] = {{"echo", builtin_echo},
                                         {"exit", builtin_exit},
                                         {"type", builtin_type},
                                         {"pwd", builtin_pwd},
                                         {"cd", builtin_cd},
                                         {NULL, NULL}};

bool is_builtin(const char *cmd) {
    for (int i = 0; dispatch[i].name; i++) {
        if (strcmp(cmd, dispatch[i].name) == 0) {
            return true;
        }
    }
    return false;
}

int main(void) {
    // Flush after every printf
    setbuf(stdout, NULL);

    while (1) {
        printf("$ ");
        char buffer[BUFFER_SIZE];
        int rc = read_line(buffer, BUFFER_SIZE, stdin);
        if (rc == 0) {
            break;
        }
        char *tokens[MAX_TOKENS];
        int token_count = tokenize(buffer, tokens, MAX_TOKENS);
        if (token_count == 0) {
            continue;
        }
        // builtins dispatch
        bool handled = false;
        for (int i = 0; dispatch[i].name != NULL; i++) {
            if (strcmp(tokens[0], dispatch[i].name) == 0) {
                dispatch[i].fn(tokens, token_count);
                handled = true;
                break;
            }
        }

        if (!handled) {
            pid_t pid = fork();
            if (pid == 0) {
                execvp(tokens[0], tokens);
                fprintf(stderr, "%s: command not found\n", tokens[0]);
                exit(1);
            } else if (pid > 0) {
                int status;
                waitpid(pid, &status, 0);
            } else {
                perror("fork");
            }
        }
    }
    return 0;
}
