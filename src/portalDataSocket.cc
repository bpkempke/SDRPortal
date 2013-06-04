#include <portalDataSocket.h>

portalDataSocket::portalDataSocket(socketType in_socket_type, int socket_num, genericSDRInterface *in_sdr_itn) : hierarchicalDataflowBlock(1, 1){
	sdr_int = in_sdr_int;

	//Create the socket that we'll be listening on...
	socket_int = new genericSocketInterface(in_socket_type, socket_num);

	//Link upper and lower 
	in_sdr_int->addLowerLevel(this);
	socket_int->addUpperLevel(this);
}

portalDataSocket::~portalDataSocket(){
	delete socket_int;
}

void portalDataSocket::dataFromUpperLevel(void *data, int num_bytes, int local_up_channel=0){
	//Data coming in from the SDR

	messageType new_message;
	new_message.buffer = data;
	new_message.num_bytes = num_bytes;

	vector<messageType> transmit_messages;
	transmit_messages.push_back(new_message);

	socket_int->dataFromUpperLevel(&transmit_messages, num_bytes);
}

void portalDataSocket::dataFromLowerLevel(void *data, int num_bytes, int local_down_channel=0){
	//Data coming in from the socket

	//First, we need to make sure data is casted correctly (data is a pointer to a vector of messages)
	vector<messageType> *in_messages = static_cast<vector<messageType>*>(data);

	//Traverse all incoming messages and forward all IQ data on to the SDR
	for(int ii=0; ii < in_messages->size(); ii++){
		sdr_int->sendIQData((*in_messages)[ii].buffer, (*in_messages)[ii].num_bytes, sdr_int->getRXPortUID(local_down_channel)); //TODO: Is local_down_channel correct???
	}
}

