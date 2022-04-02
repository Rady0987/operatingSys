#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>

struct childPID {
	pid_t pid;
	int arbitraryPid;
	//de schimbat
	friend bool operator == (const childPID& lhs, const pid_t rhs) {
		return lhs.pid == rhs;
	}
};

inline std::vector<childPID> backProcesses;
inline struct sigaction signalAction;

inline int exitFlag;


void changeDir(std::vector<std::string> command, int *status);
void inputRedirection(std::string input_file);
void outputRedirection(std::string output_file);
int shell_exec(std::vector<std::string> command, std::string input_file, std::string output_file);
void createPipeline(std::vector<std::vector<std::string>> pipeCommands, std::string input_file, std::string output_file);
int getIO(std::vector<std::string> &command, std::string *input_file, std::string *output_file);
std::vector<std::vector<std::string>> getCommandToPipe(std::vector<std::string> operators, std::vector<std::vector<std::string>> chains, int *j, int *operatorIndex);
void prepareForExec(std::vector<std::vector<std::string>> chains, std::vector<std::string> operators);
void createBackground(std::vector<std::string> command, std::string output_file);
void getJobs(); 

#endif