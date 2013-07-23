/*    SDRPortal - A generic web-based interface for SDRs
 *    Copyright (C) 2013 Ben Kempke (bpkempke@umich.edu)
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sstream>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "shellPortal.h"
#include <signal.h>
#include <sys/wait.h>

#define READ 0
#define WRITE 1

pid_t popen2(char **command, FILE **infp, FILE **outfp){
	int p_stdin[2], p_stdout[2];
	pid_t pid;
	
	if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0)
		return -1;
	
	pid = fork();
	
	if (pid < 0)
		return pid;
	else if (pid == 0){
		close(p_stdin[WRITE]);
		dup2(p_stdin[READ], READ);
		close(p_stdout[READ]);
		dup2(p_stdout[WRITE], WRITE);
		
		execvp(*command, command);
		perror("execvp");
		exit(1);
	}
	
	if (infp == NULL)
		close(p_stdin[WRITE]);
	else
		*infp = fdopen(p_stdin[WRITE],"w");
	
	if (outfp == NULL)
		close(p_stdout[READ]);
	else
		*outfp = fdopen(p_stdout[READ],"r");
	
	return pid;
}

shellPortal::shellPortal(socketType in_socket_type, int socket_num){
	socket_type = in_socket_type;

	//Create the socket that we'll be listening on...
	socket_int = new genericSocketInterface(in_socket_type, socket_num);

	//Link upper and lower 
	this->addLowerLevel(socket_int);
	socket_int->addUpperLevel(this);
}

shellPortal::~shellPortal(){
	delete socket_int;
}

void shellPortal::dataFromUpperLevel(void *data, int num_messages, int local_up_channel){
	messageType *out_messages = (messageType*)(data);
	for(int ii=0; ii < num_messages; ii++){
		dataToLowerLevel((void*)&out_messages[ii], 1);
	}
}

void shellPortal::notificationFromLower(void *in_notification){
	int closing_channel = *((int*)in_notification);
	for(unsigned int ii=0; ii < command_threads.size(); ii++){
		if(closing_channel == command_threads[ii]->down_channel){
			deleteListener(command_threads[ii]);
			ii--;
		}
	}
}

#define MAX_BUFFER 2048
static void *commandListener(void *in_args){
	cmdListenerArgs *cmd_args = (cmdListenerArgs*)in_args;
	int down_channel = cmd_args->down_channel;
	char buffer[MAX_BUFFER];
	while(fgets(buffer, MAX_BUFFER, cmd_args->out_fp) != NULL){
		messageType out_message;
		out_message.buffer = buffer;
		out_message.num_bytes = strlen(buffer);
		out_message.socket_channel = down_channel;
		cmd_args->shell_portal_ptr->dataFromUpperLevel(&out_message, 1);
	}
	cmd_args->shell_portal_ptr->deleteListener(cmd_args);
	return NULL;
}

void shellPortal::deleteListener(cmdListenerArgs *in_arg){
	kill(in_arg->pid,SIGINT);
	waitpid(in_arg->pid,NULL,WNOHANG);
	for(unsigned int ii=0; ii < command_threads.size(); ii++){
		if(in_arg == command_threads[ii])
			command_threads.erase(command_threads.begin()+ii);
	}
}

void shellPortal::dataFromLowerLevel(void *data, int num_messages, int local_down_channel){
	//Data coming in from the socket

	//First, we need to make sure data is casted correctly (data is a pointer to a vector of messages)
	messageType *in_messages = static_cast<messageType *>(data);
	std::cout << "GOT AN INCOMING MESSAGE: " << in_messages[0].buffer << std::endl;
	int down_channel = in_messages[0].socket_channel;

	//Insert historic messages into a string stream so as to easily extract lines
	static std::stringstream command_stream("");
	for(int ii=0; ii < num_messages; ii++){
		std::string in_data_string(in_messages[ii].buffer,in_messages[ii].num_bytes);
		std::cout << "APPENDING " << in_data_string << std::endl;
		command_stream << in_data_string;
	}
	std::cout << "AFTER STRINGSTREAM: " << command_stream.str() << std::endl;

	//Now parse out incoming commands
	std::string current_command;
	std::cout << "Before extraction[" << command_stream.gcount() << "]: " << command_stream.str() << std::endl;
	if(!std::getline(command_stream, current_command).fail()){
		//TODO: Not sure if stringstream should keep on growing or not?
		command_stream.clear();
		std::cout << "AFTER EXTRACTION[" << command_stream.gcount() << "]: " << command_stream.str() << std::endl;
		std::cout << "GOT A COMMAND: " << current_command << std::endl;
		//Look for the first line break (if there is one) and turn into null termination
		for(unsigned int ii=0; ii < current_command.length(); ii++)
			if(current_command[ii] == '\r' || current_command[ii] == '\n')
				current_command[ii] = 0;
		std::cout << "Now it's shrunk: " << current_command << std::endl;

		//Set up the arguments and the pthread to serve as listener to the output
		pthread_t *new_thread = new pthread_t;
		cmdListenerArgs *listener_args = new cmdListenerArgs;
		listener_args->shell_portal_ptr = this;
		listener_args->down_channel = down_channel;
		listener_args->thread_ptr = new_thread;
		
		//Split the command up into command and arguments
		std::stringstream ss(current_command);
		std::string item;
		std::vector<char*> vc;
		while(std::getline(ss,item,' ')){
			char *cur_arg = new char[item.size()+1];
			strcpy(cur_arg, item.c_str());
			vc.push_back(cur_arg);
		}
		vc.push_back(NULL);

		//Now we have a command that we should execute, execute it and create a thread to listen to stdout and pipe it to the upper level
		listener_args->pid = popen2(&vc[0], NULL, &(listener_args->out_fp));

		pthread_create(new_thread, NULL, commandListener, (void*)listener_args);
		command_threads.push_back(listener_args);

		//TODO: Add logic to kill sockets if Ctrl-C codes are experienced
	}

}

