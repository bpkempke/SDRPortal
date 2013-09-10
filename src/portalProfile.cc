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
#include <fstream>
#include <algorithm>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include "portalProfile.h"

#define COMMAND_FIFO "cmd_fifo"
#define READBACK_FIFO "rb_fifo"

#define READ 0
#define WRITE 1

bool isValidProfile(std::string profile_name){
	//This function just validates profile names based on the presence of the designated profile specification file
	profile_name += ".sdrprof";
	std::ifstream file(profile_name.c_str());
	if(!file) return false;
	else file.close();
	return true;
}

pid_t popen3(std::string command){

	//Split the command up into command and arguments
	std::stringstream ss(command);
	std::string item;
	std::vector<char*> vc;
	while(std::getline(ss,item,' ')){
		char *cur_arg = new char[item.size()+1];
		strcpy(cur_arg, item.c_str());
		vc.push_back(cur_arg);
	}
	vc.push_back(NULL);

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
		
		execvp(vc[0], &vc[0]);
		perror("execvp");
		exit(1);
	}
	
	close(p_stdin[WRITE]);
	close(p_stdout[READ]);
	
	return pid;
}


portalProfile::portalProfile(std::string profile_name){
	profile_name += ".sdrprof";
	std::ifstream file(profile_name.c_str());
	if(!file) return; //TODO: Better error checking here!

	//Profile file consists of:
	// signal processing instantiation command
	// comma-separated list of profile command names	
	//TODO: Better error checking on profile file contents
	std::string sigproc_command;
	std::getline(file, sigproc_command);

	//Read in all the csv-separated accepted command names
	//TODO: Better error checking here...
	while(file.good()){
		std::string cur_command;
		getline(file, cur_command, ',');
		commands.push_back(cur_command);
	}

	//Make FIFOs to communicate to/from the remote process
	//TODO: Probably should have more error checking here...
	mkfifo(COMMAND_FIFO, 0666);
	mkfifo(READBACK_FIFO, 0666);

	//Now instantiate the signal processing sub-process...
	sigproc_pid = popen3(sigproc_command);
}

portalProfile::~portalProfile(){
	//TODO: Is there anything else that's needed here???
	kill(sigproc_pid,SIGINT);
	waitpid(sigproc_pid,NULL,WNOHANG);
}

bool portalProfile::acceptsCommand(std::string command_name){
	//Search the vector of accepted command strings for the specified command
	std::vector<std::string>::iterator it;
	it = std::find(commands.begin(), commands.end(), command_name);
	if(it != commands.end())
		return true;
	else
		return false;
}

std::string portalProfile::sendCommand(std::string command){
	//First open the command fifo for writing, then send the command
	//   int cf_fd = open(COMMAND_FIFO, O_WRONLY);
	//   write(cf_fd, command.c_str(), command.length());
	//   close(cf_fd);
	//TODO: More error checking here as well...
	std::ofstream cf_fd(COMMAND_FIFO);
	cf_fd << command << std::endl;
	cf_fd.close();

	//Next wait for a response back
	std::string line, return_string;
	std::ifstream rb_fd(READBACK_FIFO);
	while(std::getline(rb_fd, line)){
		return_string += line;
	}
	if(rb_fd.eof())
		rb_fd.clear();
	rb_fd.close();

	return return_string;
}
