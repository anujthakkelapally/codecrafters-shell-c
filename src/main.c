#include "builtins.h"
#include "tokenizer.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define MAX_TOKENS 64

int main(void) {
    // Flush after every printf
    setbuf(stdout, NULL);

    while (1) {
        printf("$ ");
        char buffer[BUFFER_SIZE];
        int read = read_line(buffer, BUFFER_SIZE, stdin);
        if (read == 0) {
            break;
        }
        char tokens[MAX_TOKENS][MAX_TOKEN_LEN];
        ssize_t token_count = tokenize_fixed(buffer, tokens);
        if (token_count == 0) {
            continue;
        }
        char *token_array[MAX_TOKENS];
        for (int i = 0; i < MAX_TOKENS; i++) {
            token_array[i] = tokens[i];
        }
        token_array[token_count] = NULL;
        // builtins dispatch
        bool builtin_ran = dispatch_builtin(token_array, token_count);
        if (!builtin_ran) {
            pid_t pid = fork();
            if (pid == 0) {
                execvp(tokens[0], token_array);
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
