#include <portalCommandSocket.h>

portalCommandSocket::portalCommandSocket(socketType in_socket_type, int socket_num, genericSDRInterface *in_sdr_int) : hierarchicalDataflowBlock(1, 1){
	sdr_int = in_sdr_int;

	//Create the socket that we'll be listening on...
	socket_int = new genericSocketInterface(in_socket_type, socket_num);
}

portalCommandSocket::~portalCommandSocket(){
	delete socket_int;
}

void portalCommandSocket::dataFromUpperLevel(void *data, int num_bytes, int local_up_channel=0){

}

void portalCommandSocket::dataFromLowerLevel(void *data, int num_bytes, int local_down_channel=0){

}

