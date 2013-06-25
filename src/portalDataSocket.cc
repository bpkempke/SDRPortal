#include <vector>
#include "genericSDRInterface.h"
#include "portalDataSocket.h"

portalDataSocket::portalDataSocket(socketType in_socket_type, int socket_num, genericSDRInterface *in_sdr_int) : hierarchicalDataflowBlock(1, 1){
	sdr_int = in_sdr_int;
	data_type = INT8;

	//Create the socket that we'll be listening on...
	socket_int = new genericSocketInterface(in_socket_type, socket_num);

	//Link upper and lower 
	socket_int->addUpperLevel(this);
	this->addLowerLevel(socket_int);
}

portalDataSocket::~portalDataSocket(){
	delete socket_int;
}

int portalDataSocket::getPortNum(){
	return socket_int->getPortNum();
}

void portalDataSocket::setUID(int in_uid){
	uid = in_uid;
}

int portalDataSocket::getUID(){
	return uid;
}

void portalDataSocket::dataFromUpperLevel(void *data, int num_bytes, int local_up_channel){
	//Data coming in from the SDR

	messageType new_message;
	new_message.buffer = (char*)data;
	new_message.num_bytes = num_bytes;
	new_message.socket_channel = -1;

	dataToLowerLevel(&new_message, 1);
}

void portalDataSocket::dataFromLowerLevel(void *data, int num_bytes, int local_down_channel){
	//Data coming in from the socket

	//First, we need to make sure data is casted correctly (data is a pointer to a vector of messages)
	std::vector<messageType> *in_messages = static_cast<std::vector<messageType>*>(data);

	//Traverse all incoming messages and forward all IQ data on to the SDR
	for(unsigned int ii=0; ii < in_messages->size(); ii++){
		sdr_int->sendIQData((*in_messages)[ii].buffer, (*in_messages)[ii].num_bytes, uid, data_type);
	}
}

