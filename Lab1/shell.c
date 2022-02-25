/* File: shell.c */
/* Authors: Denis Garabajiu S4142551 and Radu Rusu S */
/* Description: This program is a working shell. */

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define chain_delimiter "||;&&"
#define token_delimiter " \t\a\n\r"
#define command_delimiter " \""


// Function to safely allocate memory
void *safeMalloc(int sz) {
  void *p = calloc(sz, 1);
  if (p == NULL) {
    fprintf(stderr, "Fatal error: safeMalloc(%d) failed.\n", sz);
    exit(EXIT_FAILURE);
  }
  return p;
}

// Function that executes the UNIX commands input by the user
int shell_exec(char **args, char **operators) {
    pid_t child_pid, w;
    int status;
    
    //Handling the exit command
    if (strcmp(args[0],"exit") == 0) exit(EXIT_SUCCESS);

    //Creating a child process
    child_pid = fork();
    if (args[0] == NULL) return 1;

    if (child_pid == 0) {
        if (execvp(args[0], args) < 0) {
            printf("Error: command not found!\n");
        }
        exit(EXIT_FAILURE);
    } else if (child_pid < 0) {
        printf("Forking failed");
    } else {
        // Parent process waiting for the child process to finish.
        do {
          w = waitpid(child_pid, &status, WUNTRACED);
          if (w == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
          }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 0;
}

//Parsing function to split the first input line into tokens
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

//Function used to read the input line given by the user.
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

// Parsing function to exctrat all the operators such as || && ; from the input line into separate array
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

// Function with the main loop of the shell
void shell_loop() {
    char *input_line, *input_cpy;
    char **args, **operators, **chains, **command;
    int status = 1, i = 0;

    do {
        input_line = read_line();
        input_cpy = safeMalloc((strlen(input_line) + 1) * sizeof(char));
        strcpy(input_cpy, input_line);
        
        //Parsing section
        args = split_input_line(input_line,token_delimiter);
        chains = split_input_line(input_cpy, chain_delimiter);
        operators = getOperators(args);

        while(chains[i] != NULL) {
          command = split_input_line(chains[i], command_delimiter);
          status = shell_exec(command, operators);
          free(command);
          i++;
        }
        i = 0;

        //Freeing section
        free(input_cpy);
        free(chains);
        free(operators);
        free(input_line);
        free(args);
    } while (!status);

}

int main(int argc, char **argv) {

    shell_loop();

  return EXIT_SUCCESS;
}
