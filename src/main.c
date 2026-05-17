#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
    while (token != NULL && count < max_tokens) {
        tokens[count++] = token;
        token = strtok_r(NULL, " \t\n", &saveptr);
    }
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
          else {
              char *PATH = getenv("PATH");
              char *PATH_copy = strdup(PATH);
              char *saveptr;
              char *dir = strtok_r(PATH_copy, ":", &saveptr);
              int found = 0;

              // for each PATH directory
              while (dir != NULL) {
                  char full_path[BUFFER_SIZE];
                  snprintf(full_path, sizeof(full_path), "%s/%s", dir, tokens[1]);
                  // check if file with command name exists
                  // check if file has execute permissions
                  if (access(full_path, X_OK) == 0) {
                      // If the file exists and has execute permissions, print <command> is <full_path> and stop.
                      if (found != 1) {
                          printf("%s is %s\n", tokens[1], full_path);
                      }
                      found = 1;
                      break;
                  }
                  dir = strtok_r(NULL, ":", &saveptr);
              }
              // If the file exists but lacks execute permissions, skip it and continue to the next directory.
              // If no executable is found in any directory, print <command>: not found.
              if (found == 0) {
                  printf("%s: not found\n", tokens[1]);
              }
          }
      }
      else {
          command_not_found(tokens[0]);
      }
  }

  return 0;
}
