#include <getopt.h>
#include "uhdInterface.h"
#include "rtlInterface.h"
#include "portalCommandSocket.h"
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

enum SDRType {UHD,RTL};

//GLOBALS
genericSDRInterface *sdr_interface = NULL;
portalCommandSocket *tcp_cmd_socket = NULL;
portalCommandSocket *ws_cmd_socket = NULL;

void sighandler(int sig){
	printf("sdrportal: signal %d caught...\n", sig);

	//Delete all dynamically-allocated things
	delete tcp_cmd_socket;
	delete ws_cmd_socket;
	delete sdr_interface;

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
	const char* const short_options = "s:t:w:T:W:a:";
	const struct option long_options[] = {
		{"sdr_type", required_argument, 0, 's'},
		{"tcp_cmd", required_argument, 0, 'T'},
		{"ws_cmd", required_argument, 0, 'W'},
		{"sdr_args", required_argument, 0, 'a'},
		{NULL, 0, NULL, 0}
	};


	//Open up two separate sockets, one for commands, and one for data
	int tcp_cmd_portnum = 12500;
	int ws_cmd_portnum = 12600;
	SDRType sdr_type = UHD;
	string sdr_arguments = "";

	//Parse out the arguments using getopt_long
	int next_option;
	while((next_option = getopt_long(argc, argv, short_options, long_options, NULL)) != -1){
		switch(next_option){
		case 's':
			if(!strcmp("UHD",optarg))
				sdr_type = UHD;
			else if(!strcmp("RTL",optarg))
				sdr_type = RTL;
			break;
		case 'T':
			tcp_cmd_portnum = atoi(optarg);
			break;
		case 'W':
			ws_cmd_portnum = atoi(optarg);
			break;
		case 'a':
			sdr_arguments = optarg;
			break;
		default:
			break;
		}
	}


	//Instantiate a generic SDR interface and link in with the created ports
	if(sdr_type == UHD){
		//OLD: uhdInterface usrp_instance(uhd_arguments,"","A:0","J1","J1",tx_bandwidth,rx_bandwidth,2500000000.0,2500000000.0,20.0,40.0, codec_highspeed);
		//TODOTODO: sdr_interface = new uhdInterface(sdr_arguments);
		sdr_interface = new uhdInterface(sdr_arguments,"","A:0","J1","J1",1e6,1e6,2500000000.0,2500000000.0,20.0,40.0, false);
	} else if(sdr_type == RTL){
		if(sdr_arguments.length() > 0)
			sdr_interface = new rtlInterface(atoi(sdr_arguments.c_str()));
		else
			sdr_interface = new rtlInterface(0);
	}

	//Create command sockets which will spawn data sockets if requested
	if(tcp_cmd_portnum)
		tcp_cmd_socket = new portalCommandSocket(SOCKET_TCP, tcp_cmd_portnum, sdr_interface);
	if(ws_cmd_portnum)
		ws_cmd_socket = new portalCommandSocket(SOCKET_WS_TEXT, ws_cmd_portnum, sdr_interface);

	while(1){
		sleep(30);
	}

	//Should never get here!
	//Only way to exit program is by pressing ctrl-C, which is covered in the exception handler above...
}
