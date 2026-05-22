#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

int read_line(char *buffer, size_t size, FILE* stream) {
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
    if (buffer == NULL) return 0;
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

void command_not_found(char *cmd) {
    printf("%s: command not found\n", cmd);
}

int is_builtin(char *cmd) {
    if (cmd == NULL) {
        return 0;
    }
    if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "echo") == 0 || strcmp(cmd, "type") == 0) {
        return 1;
    }
    return 0;
}

char* find_in_path(const char *path, const char *cmd, char *buffer, size_t buffer_size) {
    if (path == NULL || cmd == NULL || buffer == NULL) {
        return NULL;
    }
    char *path_copy = strdup(path);
    if (path_copy == NULL) {
        return NULL;
    }
    char *saveptr;
    char *dir = strtok_r(path_copy, ":", &saveptr);
    while (dir != NULL) {
        snprintf(buffer, buffer_size, "%s/%s", dir, cmd);
        if (access(buffer, X_OK) == 0) {
            free(path_copy);
            return buffer;
        }
        dir = strtok_r(NULL, ":", &saveptr);
    }
    free(path_copy);
    return NULL;
}

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  while (1) {
      printf("$ ");
      char buffer[1024];
      int rc = read_line(buffer, BUFFER_SIZE, stdin);
      if (rc == 0) {
          break;
      }
      int MAX_TOKENS = 64;
      char *tokens[MAX_TOKENS];
      int token_count = tokenize(buffer, tokens, MAX_TOKENS);
      if (token_count == 0) {
          break;
      }
      if (strcmp(tokens[0], "exit") == 0) {
          break;
      }
      else if (strcmp(tokens[0], "echo") == 0) {
          for (int i = 1; i < token_count; i++) {
              printf("%s", tokens[i]);
              if (i != token_count-1) {
                  printf(" ");
              }
          }
          printf("\n");
      }
      else if (strcmp(tokens[0], "type") == 0) {
          if (is_builtin(tokens[1]) == 1) {
              printf("%s is a shell builtin\n", tokens[1]);
          }
          // NOT A BUILTIN
          else {
              char *cmd_path = malloc(BUFFER_SIZE);
              cmd_path = find_in_path(getenv("PATH"), tokens[1], cmd_path, BUFFER_SIZE);
              if (cmd_path == NULL) {
                  printf("%s: not found\n", tokens[1]);
              }
              else {
                  printf("%s is %s\n", tokens[1], cmd_path);
              }
              free(cmd_path);
          }
      }
      else {
          char *cmd_path = malloc(BUFFER_SIZE);
          cmd_path = find_in_path(getenv("PATH"), tokens[0], cmd_path, BUFFER_SIZE);
          if (cmd_path == NULL) {
              command_not_found(tokens[0]);
          }
          else {
              pid_t pid = fork();
              if (pid == 0) {
                  execvp(tokens[0], tokens);
                  fprintf(stderr, "%s: command not found\n", tokens[0]);
                  exit(1);
              }
              else if (pid > 0) {
                  int status;
                  waitpid(pid, &status, 0);
              }
              else {
                  perror("fork");
              }
          }
      }
  }
  return 0;
}
