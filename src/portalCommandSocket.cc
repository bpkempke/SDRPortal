#include <vector>
#include <sstream>
#include <stdio.h>
#include "portalDataSocket.h"
#include "portalCommandSocket.h"

portalCommandSocket::portalCommandSocket(socketType in_socket_type, int socket_num, genericSDRInterface *in_sdr_int) : hierarchicalDataflowBlock(1, 1){
	sdr_int = in_sdr_int;
	socket_type = in_socket_type;

	//Create the socket that we'll be listening on...
	socket_int = new genericSocketInterface(in_socket_type, socket_num);

	//Link upper and lower 
	//TODO: Not sure if this is needed at all...
	//  in_sdr_int->addLowerLevel(this);
	this->addLowerLevel(socket_int);
	socket_int->addUpperLevel(this);
}

portalCommandSocket::~portalCommandSocket(){
	delete socket_int;
}

void portalCommandSocket::dataFromUpperLevel(void *data, int num_bytes, int local_up_channel){
	//Data coming in from the SDR

	messageType new_message;
	new_message.buffer = (char*)data;
	new_message.num_bytes = num_bytes;

	std::vector<messageType> transmit_messages;
	transmit_messages.push_back(new_message);

	socket_int->dataFromUpperLevel(&transmit_messages, num_bytes);
}

void portalCommandSocket::dataFromLowerLevel(void *data, int num_messages, int local_down_channel){
	//Data coming in from the socket

	//First, we need to make sure data is casted correctly (data is a pointer to a vector of messages)
	messageType *in_messages = static_cast<messageType *>(data);

	//Insert historic messages into a string stream so as to easily extract lines
	static std::stringstream command_stream;
	for(int ii=0; ii < num_messages; ii++){
		std::string in_data_string(in_messages[ii].buffer,in_messages[ii].num_bytes);
		command_stream << in_data_string;
	}

	//Now parse out incoming commands
	std::string current_command;
	if(!getline(command_stream, current_command).fail()){
		std::stringstream arg_stream(current_command);
		std::string command, arg1, arg2;

		//Process the command, first argument, and second argument
		arg_stream >> command;
		arg_stream >> arg1;
		arg_stream >> arg2;

		//cout << command << " " << arg1 << " " << arg2 << std::endl;

		//Now do whatever we need to do based on the received command
		//TODO: Put in some error checking here
		char response[20];
		messageType response_message;
		response_message.buffer = response;
		try{
			if(command == "NEWCHANNEL"){
				//The client wants to add a data channel connection...
				// Better create one! (and pass the random port back to the client so that he can connect to it...)
				portalDataSocket *data_socket = new portalDataSocket(socket_type, 0, sdr_int);
				data_socket->addLowerLevel(socket_int);

				int new_channel = sdr_int->addChannel(data_socket);
				response_message.num_bytes = sprintf(response_message.buffer,"%d\r\n", new_channel);
				dataToLowerLevel(&response_message, 1);
			} else if(command == "CHANNEL"){
				if(isInteger(arg1)){
					int candidate_channel = strtol(arg1.c_str(), NULL, 0);
					if(sdr_int->getNumAllocatedChannels() > candidate_channel)
						cur_channel = candidate_channel;
					else
						throw badArgumentException(badArgumentException::OUT_OF_BOUNDS, 1, arg1);
				} else
					throw badArgumentException(badArgumentException::MALFORMED, 1, arg1);
			} else if(command == "RXCHANNEL"){
				if(isInteger(arg1))
					sdr_int->bindRXChannel(strtol(arg1.c_str(), NULL, 0), cur_channel);
					//TODO: Respond with UID
				else
					throw badArgumentException(badArgumentException::MALFORMED, 1, arg1);
			} else if(command == "TXCHANNEL"){
				if(isInteger(arg1))
					sdr_int->bindTXChannel(strtol(arg1.c_str(), NULL, 0), cur_channel);
					//TODO: Respond with UID
				else
					throw badArgumentException(badArgumentException::MALFORMED, 1, arg1);
			} else {
				sdr_int->setSDRParameter(cur_channel, command, arg1);
			}
			/*//TODO: This needs to be transferred to the UHD interface code 
			if(command == "RXANT"){
				uhd_int->getUHDObject()->set_rx_antenna(arg1);
			} else if(command == "TXANT"){
				uhd_int->getUHDObject()->set_tx_antenna(arg1);
			} else {
				throw invalidCommandException();
			}*/
		} catch(badArgumentException const& e){
			std::stringstream response;
			response << "?" << e.what() << std::endl;
			dataToLowerLevel((void*)(response.str().c_str()),response.gcount());
		} catch(invalidCommandException const& e){
			std::stringstream response;
			response << "?" << e.what() << std::endl;
			dataToLowerLevel((void*)(response.str().c_str()),response.gcount());
		}

		//Query-based commands
		response_message.num_bytes = 0;
		rxtxChanInfo cur_chan_info = sdr_int->getChanInfo(cur_channel);
		if(command.substr(0,6) == "RXFREQ")
			response_message.num_bytes = sprintf(response_message.buffer,"%f\r\n",sdr_int->getRXFreq(cur_chan_info).getDouble());
		else if(command.substr(0,6) == "TXFREQ")
			response_message.num_bytes = sprintf(response_message.buffer,"%f\r\n",sdr_int->getTXFreq(cur_chan_info).getDouble());
		else if(command.substr(0,6) == "RXGAIN")
			response_message.num_bytes = sprintf(response_message.buffer,"%f\r\n",sdr_int->getRXGain(cur_chan_info).getDouble());
		else if(command.substr(0,6) == "TXGAIN")
			response_message.num_bytes = sprintf(response_message.buffer,"%f\r\n",sdr_int->getTXGain(cur_chan_info).getDouble());
		else if(command.substr(0,6) == "RXRATE")
			response_message.num_bytes = sprintf(response_message.buffer,"%d\r\n",(int)(sdr_int->getRXRate(cur_chan_info).getDouble()+0.5));
		else if(command.substr(0,6) == "TXRATE")
			response_message.num_bytes = sprintf(response_message.buffer,"%d\r\n",(int)(sdr_int->getTXRate(cur_chan_info).getDouble()+0.5));

		//Send off the response
		if(response_message.num_bytes > 0)
			dataToLowerLevel(&response_message,1);
	}

}

