#include "tokenizer.h"
#include <stdio.h>
#include <string.h>

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

void emit_token(char *buffer, size_t *buffer_index, char tokens[][MAX_TOKEN_LEN], size_t *token_index) {
    buffer[*buffer_index] = '\0';
    strncpy(tokens[(*token_index)++], buffer, MAX_TOKEN_LEN);
    *buffer_index = 0;
}

typedef enum {
    STATE_NORMAL,
    STATE_SINGLE_QUOTED,
    STATE_DOUBLE_QUOTED,
    STATE_BACKSLASH_NORMAL
} TokenizerState;

ssize_t tokenize_fixed(char *input, char tokens[][MAX_TOKEN_LEN]) {
    if (input == NULL || tokens == NULL) {
        return -1;
    }

    TokenizerState state = STATE_NORMAL;
    char buffer[MAX_TOKEN_LEN];
    size_t len = 0;

    size_t token_index = 0;
    // iterate through input as a state machine
    for (size_t i = 0; input[i] != '\0'; i++) {
        char ch = input[i];

        switch (state) {
            case STATE_NORMAL:
                if (ch == '\'') {
                    state = STATE_SINGLE_QUOTED;
                }
                else if (ch == ' ') {
                    if (len > 0) {
                        emit_token(buffer, &len, tokens, &token_index);
                    }
                }
                else if (ch == '\"') {
                    state = STATE_DOUBLE_QUOTED;
                }
                else if (ch == '\\') {
                    state = STATE_BACKSLASH_NORMAL;
                }
                else {
                    // append buffer
                    buffer[len++] = ch;
                }
                break;

            case STATE_SINGLE_QUOTED:
                if (ch == '\'') {
                    state = STATE_NORMAL;
                }
                else {
                    // append buffer
                    buffer[len++] = ch;
                }
                break;

            case STATE_DOUBLE_QUOTED:
                if (ch == '\"') {
                    state = STATE_NORMAL;
                }
                else {
                    // append buffer
                    buffer[len++] = ch;
                }
                break;

            case STATE_BACKSLASH_NORMAL:
                // append buffer
                buffer[len++] = ch;
                state = STATE_NORMAL;
                break;
        }
    }

    if (len > 0) {
        emit_token(buffer, &len, tokens, &token_index);
    }

    return (ssize_t)token_index;
}
