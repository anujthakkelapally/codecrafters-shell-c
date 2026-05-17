#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
          if (strcmp(tokens[1], "exit") == 0 || strcmp(tokens[1], "echo") == 0 || strcmp(tokens[1], "type") == 0) {
              printf("%s is a shell builtin\n", tokens[1]);
          }
          else {
              printf("%s: not found\n", tokens[1]);
          }
      }
      else {
          command_not_found(tokens[0]);
      }
  }

  return 0;
}
