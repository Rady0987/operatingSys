/* File: shell.c */
/* Authors: Denis Garabajiu S4142551 and Radu Rusu S */
/* Description: This program is a working shell. */

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>

#define chain_delimiter "||;&&"
#define token_delimiter " \t\a\r"
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

//Parsing function to split the first input line into tokens
char **split_input_line(char *input_line, char *delimiter) {
    int buffer_size = 128, index = 0;
    char **tokens = safeMalloc(buffer_size * sizeof(char));
    char *token;

    token = strtok(input_line, delimiter);

    while(token != NULL) {
        tokens[index] = token;
        index++;

        //Reallocing memory for larger inputs than 128 chars 
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

    // If we hit EOF, replace it exit command and return the buffer.
    if (c == EOF) {
      buffer[index] = 'e';
      buffer[index+1] = 'x';
      buffer[index+2] = 'i';
      buffer[index+3] = 't';
      return buffer;
    } else if (c == '\n') {
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
char **getOperators(char **args, char **input_file, char **output_file) {
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
      if(strchr(args[index], '>') != NULL) {
        operators[i] = ">";
        *output_file = args[index + 1];
        i++;
      }
      if(strchr(args[index], '<') != NULL) {
        operators[i] = "<";
        *input_file = args[index + 1];
        i++;
      }
      index++;
    }
    operators[index] = NULL;
    return operators;
}

void changeDir(char **command) {
  char str[100];
  int returnValue;
  if (command[1] == NULL) {
    printf("Error: cd requires folder to navigate to!\n");
    return;
  }
  
  strcpy(str, command[1]);

  int i = 2;
  while (command[i] != NULL) {
    strcat(str, " ");
    strcat(str, command[i]);
    i++;
  }

  returnValue = chdir(str);
  if (returnValue == -1) {
    printf("Error: cd directory not found!\n");
  }
}

// Function that executes the UNIX commands input by the user
int shell_exec(char **chains, char **operators, char *input_file, char *output_file) {
  pid_t child_pid, w;
  int status;
  int commandIndex = 0, operatorIndex = 0;
  char **command;

  //Looping over all stored chains
  while (chains[commandIndex] != NULL ) {

    bool isCorrectOperator = true;

    //Setting the chain input the right format for execvp
    command = split_input_line(chains[commandIndex], command_delimiter);

          // int j = 0;
      // while (command[j] != NULL)
      // {
      //   printf("command[%d] = %s\n", j, command[j]);
      //   j++;
      // }


    if (input_file != NULL && output_file != NULL) {
      // Check if input and output files are not equal.
      if (strcmp(input_file, output_file) == 0) {
        printf("Error: input and output files cannot be equal!\n");
      }
      return(0);
    }

    int i = 0;
    while (command[i] != NULL) {
      if (strcmp(command[i], "<") == 0 || strcmp(command[i], ">") == 0) {
        command[i] = NULL;
      }
      i++;
    }

    //Checking the operator starting with the 2nd chain and taking into 
    //consideration the exit code of the previous command
    if (operators[operatorIndex] != NULL) {
      if ((strcmp(operators[operatorIndex], ";") == 0 || strcmp(operators[operatorIndex],"\n") == 0) && commandIndex != 0) {
        operatorIndex++;
      } else if (strcmp(operators[operatorIndex], "||") == 0 && commandIndex != 0) {
        if (WEXITSTATUS(status) == 0) {
          isCorrectOperator = false;
        }
        operatorIndex++;
      } else if (strcmp(operators[operatorIndex], "&&") == 0 && commandIndex != 0) {
        if (WEXITSTATUS(status) != 0) {
          isCorrectOperator = false;
        }
        operatorIndex++;
      }
    }
    commandIndex++;

    if (isCorrectOperator == false) {
      free(command);
      continue;
    }

    //Handling the exit command
    if (strcmp(command[0], "exit") == 0) {
      free(command);
      return(1);
    }

    //Handling the cd command
    if(strcmp(command[0], "cd") == 0) {
      changeDir(command);
      commandIndex++;
      free(command);
      return(0);
    }

    //Creating a child process
    child_pid = fork();
    if (command[0] == NULL) return 1;

    if (child_pid == 0) {

      if (input_file != NULL || output_file != NULL) {
        if (strcmp(operators[operatorIndex], "<") == 0 && commandIndex != 0) {
          int fd0;
          if ((fd0 = open(input_file, O_RDONLY, 0)) < 0) {
            printf("Couldn't open input file\n");
            exit(0);
          }           
          dup2(fd0, STDIN_FILENO);
          close(fd0);
          operatorIndex++;
        } else if (strcmp(operators[operatorIndex], ">") == 0 && commandIndex != 0) {
          
          int fd1 = open(output_file, O_WRONLY | O_TRUNC | O_CREAT, 0600);
          if ((fd1 < 0)) {
            printf("Couldn't open the output file\n");
            exit(0);
          }           
          dup2(fd1, STDOUT_FILENO); ///////problema
          close(fd1);
          operatorIndex++;
        }
        
      }

      //Command execution
      if (execvp(command[0], command) < 0) {
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
    free(command);
  }
  
  return 0;
}

// Function with the main loop of the shell
void shell_loop() {
  char *input_line, *input_cpy;
  char **args, **operators, **chains;
  int status = 1;
  char *input_file = NULL, *output_file = NULL;

  // Loop to do input reading, parsing and command executing
  do {
    input_line = read_line();
    input_cpy = safeMalloc((strlen(input_line) + 1) * sizeof(char));
    strcpy(input_cpy, input_line);
        
    //Parsing section
    args = split_input_line(input_line, token_delimiter);
    chains = split_input_line(input_cpy, chain_delimiter);
    operators = getOperators(args, &input_file, &output_file);
    //printf(" IN %s OUT %s\n", input_file, output_file);
    // int i = 0;
    // while(chains[i] != NULL) {
    //   printf("chain[%d] = %s\n", i, chains[i]);
    //   i++;
    // }

    status = shell_exec(chains, operators, input_file, output_file);

    //Freeing section
    input_file = NULL;
    output_file = NULL;
    free(input_line);
    free(input_cpy);
    free(chains);
    free(args);
    free(operators);

  } while (!status);
}

int main(int argc, char **argv) {

    shell_loop();

  return EXIT_SUCCESS;
}
