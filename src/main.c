#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  printf("$ ");
  // setup a buffer
  char buffer[1024];
  // write from file descriptor into buffer
  fgets(buffer, 1024, stdin);
  // clean the buffer for string functions
  char *ptr = strchr(buffer, '\n');
  *ptr = '\0';
  // print the buffer
  printf("%s: command not found\n", buffer);


  return 0;
}
