#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  while (1) {
      printf("$ ");

      char buffer[1024];
      if (fgets(buffer, 1024, stdin) == NULL) {
          break;
      }

      // clean the buffer for string functions
      char *ptr = strchr(buffer, '\n');
      if (ptr != NULL) {
          *ptr = '\0';
      }

      printf("%s: command not found\n", buffer);
  }

  return 0;
}
