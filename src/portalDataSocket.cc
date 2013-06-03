#include <portalDataSocket.h>

portalDataSocket::portalDataSocket(socketType in_socket_type, int socket_num, genericSDRInterface *in_sdr_itn) : hierarchicalDataflowBlock(1, 1){
	sdr_int = in_sdr_int;

	//Create the socket that we'll be listening on...
	socket_int = new genericSocketInterface(in_socket_type, socket_num);
}

portalDataSocket::~portalDataSocket(){
	delete socket_int;
}

void portalDataSocket::dataFromUpperLevel(void *data, int num_bytes, int local_up_channel=0){

}

void portalDataSocket::dataFromLowerLevel(void *data, int num_bytes, int local_down_channel=0){

}

