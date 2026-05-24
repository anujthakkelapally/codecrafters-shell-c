#pragma once
#include <stddef.h>
#include <stdio.h>

#define MAX_TOKEN_LEN 64

int read_line(char *buffer, size_t size, FILE *stream);
int tokenize(char *buffer, char **tokens, int max_tokens);
ssize_t tokenize_fixed(char *input, char tokens[][MAX_TOKEN_LEN]);
