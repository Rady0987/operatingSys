/* File: shell.c */
/* Authors: Denis Garabajiu S4142551 and Radu Rusu S */
/* Description: This program is a working shell. */

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define token_delimiter " \t\r\n\a"

void *safeMalloc(int sz) {
  void *p = calloc(sz, 1);
  if (p == NULL) {
    fprintf(stderr, "Fatal error: safeMalloc(%d) failed.\n", sz);
    exit(EXIT_FAILURE);
  }
  return p;
}

int exit_shell(char **args) {
  return 0;
}

int shell_exec(char **args) {
    pid_t child_pid;
    int status;

    child_pid = fork();

    if (args[0] == NULL) return 1;

    if (child_pid == 0) {
        if (execvp(args[0], args) < 0) {
            printf("Command not found");
        }
        exit(EXIT_FAILURE);
    } else if (child_pid < 0) {
        printf("Forking failed");
    } else {
        waitpid(child_pid, &status, WUNTRACED);
    }
    return 1;
}

char **split_input_line(char *input_line) {
    int buffer_size = 128, index = 0;
    char **tokens = safeMalloc(buffer_size * sizeof(char));
    char *token;

    token = strtok(input_line, token_delimiter);

    while(token != NULL) {
        tokens[index] = token;
        index++;
        if (buffer_size <= index) {
            buffer_size += 128;
            tokens = realloc(tokens, buffer_size * sizeof(char*));
            if (!tokens) {
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, token_delimiter);
    }
    tokens[index] = NULL;
    return tokens;
}

// char **split_input_commands(char *input_line) {
//   int buffer_size = 128, index = 0, inputSize, j = 0, buffer_size_token = 128;
//   char **tokens = safeMalloc(buffer_size * sizeof(char));
//   char *token;
 
//   inputSize = strlen(input_line);
//   for(int i = 0; i < inputSize; i++) {
    
//   // echo 9 && echo a
//     if (input_line[i] != '&' || input_line[i] != '|' || input_line[i] != ';' || input_line[i] != '\n') {
//       token[j] = input_line[i];
//       j++;
//     }
//     i++;
//     if (input_line[i] == '&' || input_line[i] == '|' || input_line[i] == ';' || input_line[i] == '\n') {
//       token[j] = '\0';
//       tokens[index] = token;
//       j = 0;
//       index++;
//     }
//   }

// }

char *read_line() {
  int buffer_size = 1024, index = 0;
  char *buffer = safeMalloc(buffer_size * sizeof(char));
  int c;

  while (1) {
    c = getchar();

    // If we hit EOF, replace it with a null character and return.
    if (c == EOF || c == '\n') {
      buffer[index] = '\0';
      return buffer;
    } else {
      buffer[index] = c;
    }
    index++;

    // If we have exceeded the buffer, reallocate.
    if (index >= buffer_size) {
      buffer_size += 1024;
      buffer = realloc(buffer, buffer_size);

      if (!buffer) {
        fprintf(stderr, "Shell: Buffer reallocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

void shell_loop() {

    char *input_line;
    char **args;
    int status = 1;

    do {
        getenv("PATH");
        printf("> ");
        input_line = read_line();
        args = split_input_line(input_line);
        status = shell_exec(args);
        printf("%s\n", input_line); //Debug to see the current input line
        //printf("%s %s %s\n", args[0], args[1], args[2]);// Debug to see args
        free(input_line);
        free(args);
    } while (status);

}

int main(int argc, char **argv) {

    shell_loop();

  return EXIT_SUCCESS;
}
