
#include "functions.h"

using namespace std;

int pidCounter = 1;
pid_t foreProcess = -1;

// Function to implement cd Unix command
void changeDir(vector<string> command, int *status) {
  string str;
  int returnValue;
  if (command.size() < 2) {
    printf("Error: cd requires folder to navigate to!\n");
    *status = 256;
    return;
  }
  
  str = command[1];

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
void createPipeline(vector<vector<string> > pipeCommands, string input_file, string output_file) {
  pid_t child_pid;
  int pipeIndex = 0, child;
  int defaultOut = dup(1);
  int numberOfCommands = pipeCommands.size();
  int numberOfPipes = (2 * numberOfCommands - 2) / 2; 
  int pipes[2 * numberOfPipes];

  for(int i = 0; i < numberOfCommands; i++) {
    for(int j = 0; j < pipeCommands[i].size(); j++) {
      if(pipeCommands[i][j].compare(">") == 0 || pipeCommands[i][j].compare("<") == 0) {
        pipeCommands[i].erase(pipeCommands[i].begin() + j);
        pipeCommands[i].erase(pipeCommands[i].begin() + j);
        j -= 2;
      }
    }
  }
  
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

      char* arguments[pipeCommands[commandIndex].size() + 1];
      for (int i = 0; i < pipeCommands[commandIndex].size(); i++) {
        arguments[i] = (char *) pipeCommands[commandIndex][i].c_str();
      }
      arguments[pipeCommands[commandIndex].size()] = NULL;

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

  for (int i = 0; i < command.size(); i++) {

    if (command[i].compare("<") == 0){
      if (i + 1 < command.size()) {
        *input_file = command[i + 1];
        command.erase(command.begin() + i);
        command.erase(command.begin() + i);
        status = 1;
      } else {
        status = -1;
      }
    }

    if (command[i].compare(">") == 0) {
      if (i + 1 < command.size()) {
        *output_file = command[i + 1];
        command.erase(command.begin() + i);
        command.erase(command.begin() + i);
        status = 1;
      } else {
        status = -1;
      }
    }
  //             for(int j = 0; j < command.size(); j++){
  //         cout << command[j] << ", ";
  //       }
  //       cout << "\n";
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

void createBackground(vector<string> command, string output_file) {
  pid_t child_pid, w;
  int status;

  child_pid = fork();

  if (child_pid != 0) { //create custom pid object and push to bg vector
      struct childPID c;
      c.pid = child_pid;
      c.arbitraryPid = pidCounter;
      pidCounter++;
      backProcesses.push_back(c);
   }

  if (child_pid == 0 ) {
    char *arguments[command.size() + 1];
    for (int i = 0; i < command.size(); i++) {
      arguments[i] = (char *) command[i].c_str();
    }
    arguments[command.size()] = NULL;

    if (!output_file.empty()) {
      int writeDescriptor = open (output_file.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0600);
      close(STDOUT_FILENO);
      dup2(writeDescriptor, STDOUT_FILENO);
      execvp(arguments[0], arguments);
    } else {
      execvp(arguments[0], arguments);
    }
  } else if (child_pid == -1) {
      // Parent process waiting for the child process to finish.
      // do {
      //   w = waitpid(child_pid, &status, WUNTRACED);
      //   if (w == -1) {
      //     perror("waitpid");
      //     exit(EXIT_FAILURE);
      //   }
      // } while (!WIFEXITED(status) && !WIFSIGNALED(status));
      do {
         w = waitpid(child_pid, &status, WUNTRACED);
      } while (w != child_pid);
  }
}

//checks if a string only contains numbers (and thus can be represented as an int)
bool isNumber(string s) {
    for (int i = 0; i < s.size(); i++) {
        if (isdigit(s[i]) == 0) return false;
    }
    return true;
}

int handleKill(vector<string> parsedLine, int &i) {
	if (parsedLine.size() - i != 1 && isNumber(parsedLine[i+1])) { //check if we have kill and a pid that is a number
		bool check = true; //check if pid actually exists; if this is not set to false, we print an error later on
		i++;
		for (int j = backProcesses.size() - 1; j >= 0; j--) { //go over bg processes and close if we find match
			if (backProcesses[j].arbitraryPid == stoi(parsedLine[i])) {
				kill(backProcesses[j].pid, SIGTERM);
				check = false;
				break;
			}
		}
		if (check) cout << "Error: this index is not a background process!" << endl; 
	} else if (parsedLine.size() - i == 1) { //we only have kill, no pid
		cout << "Error: command requires an index!" << endl;
		parsedLine.clear();
		return 1;
	} else if (parsedLine.size()-i != 1 && !isNumber(parsedLine[i+1])) { //we have a pid, but it is not a number
		cout << "Error: invalid index provided!" << endl;
		parsedLine.clear();
		return 1;
	}
	return 0;
}

void endBackgroundProcess(int sig, siginfo_t *signalInfo, void *uap) {
  int status;
  pid_t executing = waitpid(signalInfo->si_pid, &status, WNOHANG); 
  if (executing == signalInfo->si_pid) { 
	  kill(signalInfo->si_pid, SIGTERM);
	  waitpid(signalInfo->si_pid, &status, 0);
	  for (int i = 0; i < backProcesses.size(); i++) {
	  	if (backProcesses[i].pid == signalInfo->si_pid) 
        backProcesses.erase(remove(backProcesses.begin(), backProcesses.end(), signalInfo->si_pid), backProcesses.end());
	  }
	}
}

void ctrlCPressed(int sig) {
  // In case background processes are active, return error 
  if (foreProcess == -1 && !backProcesses.empty()) { 
		printf("Error: there are still background processes running!\n");
		signal(SIGINT, SIG_IGN);
	} 

  // In case there are foreground processes and no background processes 
	if (foreProcess != -1 && !backProcesses.empty()) { 
		kill(foreProcess, SIGTERM);
	} 

  // In case foreground and background are empty, just exit
  if (foreProcess == -1 && backProcesses.empty()) {
		exit(0);
	}
}

void getJobs() {
  if (!backProcesses.empty()) {
    for (int i = backProcesses.size() - 1; i >= 0; i--) {
			printf("Process running with index %d\n", backProcesses[i].arbitraryPid);
		}
	} else {
		printf("No background processes!\n");
	}
}

void prepareForExec(vector<vector<string>> chains, vector<string> operators) {
  int commandIndex = 0, operatorIndex = 0, status, IOstatus;
  vector<vector<string>> commandToPipe;
  int isPipe = 0;
  string input_file, output_file;

  // for(int j = 0; j < chains.size(); j++) {
  //   cout << "chain " << endl;
  //   for(int i = 0; i < chains[j].size(); i++) {
  //       cout << chains[j][i] << " ";
  //     }
  //   cout << endl;
  // }
  signalAction.sa_flags = SA_NOCLDSTOP | SA_SIGINFO | SA_RESTART; 
  signalAction.sa_sigaction = endBackgroundProcess;
  sigaction(SIGCHLD, &signalAction, NULL);

  signal(SIGINT, ctrlCPressed);

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
        isPipe = 1;
        commandToPipe = getCommandToPipe(operators, chains, &j, &operatorIndex);
        commandToPipe.push_back(chains[j]);
      } else if (operators[operatorIndex].compare("&") == 0) {
        isPipe = 2;

      }
    }

    if (isCorrectOperator == false) {
      continue;
    }

    //Handling the exit command
    if (chains[j].size() > 0 && chains[j][0].compare("exit") == 0) {
      exitFlag = 1;
      return;
    }

    //Handling the jobs command
    if (chains[j].size() > 0 && chains[j][0].compare("jobs") == 0) {
      getJobs();
      continue;
    }

    //Handling the kill command
    if (chains[j].size() > 0 && chains[j][0].compare("kill") == 0) {
      if (handleKill(chains[j], j) == 1)
        break;
      continue;
    }

    //Handling the cd command
    if(chains[j].size() > 0 && chains[j][0].compare("cd") == 0) {
      changeDir(chains[j], &status);
      continue;
    }

    input_file.clear();
    output_file.clear();

    IOstatus = getIO(chains[j], &input_file, &output_file);

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

    if (isPipe == 1) {
      createPipeline(commandToPipe, input_file, output_file);
    } else if(isPipe == 0){
      status = shell_exec(chains[j], input_file, output_file);
    } else if(isPipe == 2) {
      createBackground(chains[j], output_file);
    }
    isPipe = 0;
  }
}

