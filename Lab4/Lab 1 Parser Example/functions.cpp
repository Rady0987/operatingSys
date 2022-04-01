
#include "functions.h"

using namespace std;

// Function to implement cd Unix command
void changeDir(vector<string> command, int *status) {
  string str;
  int returnValue;
  if (command[1].empty()) {
    printf("Error: cd requires folder to navigate to!\n");
    *status = 256;
    return;
  }
  
  str = command[1];

  int i = 2;
  while (!command[i].empty()) {
    str.append(" ");
    str.append(command[i]);
    i++;
  }

  returnValue = chdir(str.c_str());
  if (returnValue == -1) {
    printf("Error: cd directory not found!\n");
    *status = 256;
    return;
  } 
  *status = 0;
}

//Function to redirect the input
void inputRedirection(string input_file) {
  int fd0;
  if ((fd0 = open(input_file.c_str(), O_RDONLY, 0)) < 0) {
    printf("Failed to open input file\n");
    exit(EXIT_FAILURE);
  }           
  dup2(fd0, STDIN_FILENO);
  close(fd0);
}

//Function to redirect the output
void outputRedirection(string output_file) { 
  int fd1 = open(output_file.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0600);
  if ((fd1 < 0)) {
    printf("Failed to open output file\n");
    exit(EXIT_FAILURE);
  }           
  dup2(fd1, STDOUT_FILENO); 
  close(fd1);
}

int shell_exec(vector<string> command, string input_file, string output_file) {
  pid_t child_pid, w;
  int status, defaultOut = dup(1);

  char *arguments[command.size() + 1];
  for (int i = 0; i < command.size(); i++) {
    arguments[i] = (char *) command[i].c_str();
  }
  arguments[command.size()] = NULL;

  //Creating a child process
    child_pid = fork();
    if (arguments[0] == NULL) exit(EXIT_FAILURE);

    if (child_pid == 0) {
      // Child process
      if (!input_file.empty()) 
        inputRedirection(input_file);
      
      if (!output_file.empty())
        outputRedirection(output_file);

      //Command execution
      if (execvp(arguments[0], arguments) < 0) {
        
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
    dup2(defaultOut, 1);
    close(defaultOut);
    return status;
}

//Function to implement the pipeline functionallity
void createPipeline(vector<vector<string>> pipeCommands, string input_file, string output_file) {
  pid_t child_pid;
  int pipeIndex = 0, child;
  int defaultOut = dup(1);
  int numberOfCommands = pipeCommands.size();
  int numberOfPipes = (2 * numberOfCommands - 2) / 2; 
  int pipes[2 * numberOfPipes];
  
  // Opening pipes
  for (int i = 0; i < numberOfPipes; i++) {
    pipe(2 * i + pipes);
  }

  // Pipe connection with the commands
  for (int commandIndex = 0; commandIndex < numberOfCommands; commandIndex++) {
    child = fork();

    if (child == 0) {

      if(commandIndex + 1 != numberOfCommands) {
        if(dup2(pipes[pipeIndex + 1], 1) < 0) {
          exit(EXIT_FAILURE);
        }
      }

      if (commandIndex != 0) {
        if (dup2(pipes[pipeIndex - 2], 0) < 0) {
          exit(EXIT_FAILURE);
        }
      }


      if (input_file.empty() == false && commandIndex == 0) {
        int readDescriptor;
        readDescriptor = open(input_file.c_str(), O_RDONLY);
        if(readDescriptor < 0){ 
          return;
        }
        dup2(readDescriptor, STDIN_FILENO); 
        close(readDescriptor);
      }

      if (output_file.empty() == false && commandIndex + 1 == pipeCommands.size()) {
        int writeDescriptor;
        writeDescriptor = open(output_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if(writeDescriptor < 0){ 
          return;
        }
        dup2(writeDescriptor, STDOUT_FILENO); 
        close(writeDescriptor);
      }
      
      for(int j = 0; j < numberOfPipes * 2; j++) {
        close(pipes[j]);
      }

      char* arguments[pipeCommands[commandIndex].size() + 1];
      for (int i = 0; i < pipeCommands[commandIndex].size(); i++) {
        arguments[i] = (char *) pipeCommands[commandIndex][i].c_str();
      }
      arguments[pipeCommands[commandIndex].size()] = NULL;

      if (execvp(arguments[0], arguments) < 0) {
        printf("Error: command not found!\n");
        exit(EXIT_FAILURE);
      }
    } else if (child < 0) {
      exit(EXIT_FAILURE);
    }
    
    pipeIndex = pipeIndex + 2;
  }
 
  dup2(defaultOut, 1);
  close(defaultOut);

  for(int i = 0; i < 2 * numberOfPipes; i++) {
    close(pipes[i]);
  }

  for(int i = 0; i < numberOfPipes + 1; i++) {
    wait(&child_pid);
  }
}

int getIO(vector<string> &command, string *input_file, string *output_file) {
  int status = 0;
        //   for(int j = 0; j < command.size(); j++){
        //   cout << command[j] << ", ";
        // }
        // cout << "\n";
  for (int i = 0; i < command.size(); i++) {
    if (command[i].compare(">") == 0) {
      cout << "bleeaaaaaa\n";
      if (i + 1 < command.size()) {
        *output_file = command[i + 1];
        command.erase(command.begin() + i);
        command.erase(command.begin() + i);
        
        for(int j = 0; j < command.size(); j++){
          cout << command[j] << " ";
        }
        status = 1;
      } else {
        status = -1;
      }
    }

    if (command[i].compare("<") == 0){
      if (i + 1 < command.size()) {
        *input_file = command[i + 1];
        command.erase(command.begin() + i + 1);
        command.erase(command.begin() + i);
        status = 1;
      } else {
        status = -1;
      }
    }
  }
  return status;
}

vector<vector<string>> getCommandToPipe(vector<string> operators, vector<vector<string>> chains, int *j, int *operatorIndex) {
  int numOperators = operators.size();
  vector<vector<string>> commandsToPipe;
  while (numOperators > *operatorIndex && operators[*operatorIndex].compare("|") == 0) {
    commandsToPipe.push_back(chains[*j]);
    (*j)++;
    (*operatorIndex)++;
    if(*j == chains.size()) {
      if(numOperators <= *operatorIndex) {
        (*j)--;
      } else {
        printf("Error: invalid syntax!\n");
        exit(EXIT_FAILURE);
      }
    }
  }
  return commandsToPipe;
}

void prepareForExec(vector<vector<string>> chains, vector<string> operators) {
  int commandIndex = 0, operatorIndex = 0, status, IOstatus;
  vector<vector<string>> commandToPipe;
  bool isPipe = false;
  string input_file, output_file;

  for (int j = 0; j < chains.size(); j++) {

    bool isCorrectOperator = true;

    //Checking the operator starting with the 2nd chain and taking into 
    //consideration the exit code of the previous command
    if (operatorIndex < operators.size()) {
      if ((operators[operatorIndex].compare(";") == 0 || operators[operatorIndex].compare("\n") == 0) && commandIndex != 0) {
        operatorIndex++;
      } else if (operators[operatorIndex].compare("||") == 0 && j != 0) {
        if (WEXITSTATUS(status) == 0) {
          isCorrectOperator = false;
        }
        operatorIndex++;
      } else if (operators[operatorIndex].compare("&&") == 0 && j != 0) {
        if (WEXITSTATUS(status) != 0) {
          isCorrectOperator = false;
        }
        operatorIndex++;
      } else if (operators[operatorIndex].compare("|") == 0) {
        isPipe = true;
        commandToPipe = getCommandToPipe(operators, chains, &j, &operatorIndex);
        commandToPipe.push_back(chains[j]);
      }
    }

    if (isCorrectOperator == false) {
      continue;
    }

    //Handling the exit command
    if (chains[j][0].compare("exit") == 0) {
      return;
    }

    //Handling the cd command
    if(chains[j][0].compare("cd") == 0) {
      changeDir(chains[j], &status);
      continue;
    }

    input_file.clear();
    output_file.clear();
    // ??????????? verifica daca <, > + denumirea fileului se sterge
    IOstatus = getIO(chains[j], &input_file, &output_file);
    // cout << input_file << "\n";
    // cout << "out " << output_file << "\n";
    // cout << "status " << status << "\n";

    if (IOstatus == 1) {
      // Check if input and output files are not equal.
      if (input_file.compare(output_file) == 0) {
        printf("Error: input and output files cannot be equal!\n");
        status = 256;
        continue;
      }
    } else if (IOstatus == -1){
      printf("Error: invalid syntax!\n");
      status = 256;
      continue;
    }

    if (isPipe) {
    //       for (int i = 0; i < commandToPipe.size(); i++) {
    //     for (int k = 0; k < commandToPipe[k].size(); k++){
    //         cout << commandToPipe[i][k] << ", ";
    //         if (k == commandToPipe[i].size() - 1)
    //             cout << "|";
    //     }
    // }
    // cout << "\n";


      createPipeline(commandToPipe, input_file, output_file);
    } else {
      status = shell_exec(chains[j], input_file, output_file);
    }
    isPipe = false;
  }
}

