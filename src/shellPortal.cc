#include <sstream>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "shellPortal.h"

shellPortal::shellPortal(socketType in_socket_type, int socket_num) : hierarchicalDataflowBlock(1, 1){
	socket_type = in_socket_type;

	//Create the socket that we'll be listening on...
	socket_int = new genericSocketInterface(in_socket_type, socket_num);

	//Link upper and lower 
	//TODO: Not sure if this is needed at all...
	//  in_sdr_int->addLowerLevel(this);
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

#define MAX_BUFFER 2048
static void *commandListener(void *in_args){
	cmdListenerArgs *cmd_args = (cmdListenerArgs*)in_args;
	int down_channel = cmd_args->down_channel;
	char buffer[MAX_BUFFER];
	while(fgets(buffer, MAX_BUFFER, cmd_args->command_fp) != NULL){
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
	pclose(in_arg->command_fp);
}

void shellPortal::dataFromLowerLevel(void *data, int num_messages, int local_down_channel){
	//Data coming in from the socket

	//First, we need to make sure data is casted correctly (data is a pointer to a vector of messages)
	messageType *in_messages = static_cast<messageType *>(data);
	int down_channel = in_messages[0].socket_channel;

	//Insert historic messages into a string stream so as to easily extract lines
	static std::stringstream command_stream;
	for(int ii=0; ii < num_messages; ii++){
		std::string in_data_string(in_messages[ii].buffer,in_messages[ii].num_bytes);
		command_stream << in_data_string;
	}

	//Now parse out incoming commands
	std::string current_command;
	if(!getline(command_stream, current_command).fail()){
		//Now we have a command that we should execute, execute it and create a thread to listen to stdout and pipe it to the upper level
		FILE *new_shell_output = new FILE;

		//Look for the first line break (if there is one) and turn into null termination
		for(unsigned int ii=0; ii < current_command.length(); ii++)
			if(current_command[ii] == '\r' || current_command[ii] == '\n')
				current_command[ii] = 0;

		//Run the command
		new_shell_output = popen(current_command.c_str(), "r");
		//new_shell_output = popen("ls", "r");

		//Set up the arguments and the pthread to serve as listener to the output
		pthread_t *new_thread = new pthread_t;
		cmdListenerArgs *listener_args = new cmdListenerArgs;
		listener_args->shell_portal_ptr = this;
		listener_args->down_channel = down_channel;
		listener_args->command_fp = new_shell_output;
		listener_args->thread_ptr = new_thread;

		pthread_create(new_thread, NULL, commandListener, (void*)listener_args);
		command_threads.push_back(listener_args);

		//TODO: Add logic to kill sockets if Ctrl-C codes are experienced
	}

}

