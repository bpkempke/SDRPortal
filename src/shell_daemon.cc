#include <getopt.h>
#include "shellPortal.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <iostream>

using namespace std;

shellPortal *global_tcp_shell_portal = NULL;
shellPortal *global_ws_shell_portal = NULL;

void sighandler(int sig){
	printf("sdrportal: signal %d caught...\n", sig);

	//Delete all dynamically-allocated things
	delete global_tcp_shell_portal;
	delete global_ws_shell_portal;

	exit(1);
}

int main(int argc, char *argv[]){

	//In this thread, we'd like to ignore all SIGPIPE signals, and handle them separately
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGPIPE);
	pthread_sigmask(SIG_BLOCK, &set, NULL);

	struct sigaction new_action, old_action;
	/* Set up the structure to specify the new action. */
	new_action.sa_handler = sighandler;
	sigemptyset (&new_action.sa_mask);
	new_action.sa_flags = 0;
	
	//Set up signal handlers
	sigaction (SIGINT, NULL, &old_action);
	if (old_action.sa_handler != SIG_IGN)
		sigaction (SIGINT, &new_action, NULL);
		sigaction (SIGHUP, NULL, &old_action);
	if (old_action.sa_handler != SIG_IGN)
		sigaction (SIGHUP, &new_action, NULL);
		sigaction (SIGTERM, NULL, &old_action);
	if (old_action.sa_handler != SIG_IGN)
		sigaction (SIGTERM, &new_action, NULL);

	//Set up getopt
	const char* const short_options = "p:w:";
	const struct option long_options[] = {
		{"tcp_port_num", required_argument, 0, 'p'},
		{"ws_port_num", required_argument, 0, 'w'},
		{NULL, 0, NULL, 0}
	};


	//Open up two separate sockets, one websocket, one tcp
	int tcp_portnum = 12700;
	int ws_portnum = 12800;

	//Parse out the arguments using getopt_long
	int next_option;
	while((next_option = getopt_long(argc, argv, short_options, long_options, NULL)) != -1){
		switch(next_option){
		case 'p':
			tcp_portnum = atoi(optarg);
			break;
		case 'w':
			ws_portnum = atoi(optarg);
			break;
		default:
			break;
		}
	}


	//Instantiate a generic SDR interface and link in with the created ports
	global_tcp_shell_portal = new shellPortal(SOCKET_TCP, tcp_portnum);
	global_ws_shell_portal = new shellPortal(SOCKET_WS, ws_portnum);

	//Now just wait while we accept commands!  Don't need to do anything in the main loop...
	while(1){
		sleep(30);
	}

	//Should never get here!
	//Only way to exit program is by pressing ctrl-C, which is covered in the exception handler above...
}
