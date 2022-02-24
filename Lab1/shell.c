/* File: shell.c */
/* Authors: Denis Garabajiu S4142551 and Radu Rusu S */
/* Description: This program is a working shell. */

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define chain_delimiter "||;&&"
#define token_delimiter " \t\r\n\a"
#define command_delimiter " \""


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

char **split_input_line(char *input_line, char *delimiter) {
    int buffer_size = 128, index = 0;
    char **tokens = safeMalloc(buffer_size * sizeof(char));
    char *token;

    token = strtok(input_line, delimiter);

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
        token = strtok(NULL, delimiter);
    }
    tokens[index] = NULL;
    return tokens;
}

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

char **getOperators(char **args) {
    int buffer_size = 128, index = 0, i = 0;
    char **operators = safeMalloc(buffer_size * sizeof(char));
    while(args[index] != NULL) {
      if(strcmp(args[index], "&&") == 0) {
        operators[i] = "&&";
        i++;
      }
      if(strcmp(args[index], "||") == 0) {
        operators[i] = "||";
        i++;
      }
      if(strchr(args[index], ';') != NULL) {
        operators[i] = ";";
        i++;
      }
      index++;
    }
    operators[index] = NULL;
    return operators;
}

void shell_loop() {

    char *input_line, *input_cpy;
    char **args;
    char **operators;
    char **chains;
    char **command;
    int status = 1;
    int i = 0;
    do {
        getenv("PATH");
        printf("> ");
        input_line = read_line();
        input_cpy = safeMalloc(strlen(input_line) * sizeof(char));
        strcpy(input_cpy, input_line);
        args = split_input_line(input_line,token_delimiter);
        chains = split_input_line(input_cpy, chain_delimiter);
        operators = getOperators(args);

        while(chains[i] != NULL) {
          //printf("%s\n", chains[i]);
          command = split_input_line(chains[i], command_delimiter);
          //printf("%s, \n",command[i]);
          status = shell_exec(command);
          free(command);
          i++;
        }
        i = 0;
        //printf("%s\n", input_line); //Debug to see the current input line
        // while(command[i] != NULL) {
        //   printf("%s, ", command[i]);// Debug to see args
        //   i++;
        // }
        // i = 0;
        // while(operators[i] != NULL) {
        //   printf("%s, ", operators[i]);// Debug to see args
        //   i++;
        // }
        free(input_cpy);
        free(chains);
        free(operators);
        free(input_line);
        free(args);
    } while (status);

}

int main(int argc, char **argv) {

    shell_loop();

  return EXIT_SUCCESS;
}
