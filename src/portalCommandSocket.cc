#include <portalCommandSocket.h>

portalCommandSocket::portalCommandSocket(socketType in_socket_type, int socket_num, genericSDRInterface *in_sdr_int) : hierarchicalDataflowBlock(1, 1){
	sdr_int = in_sdr_int;

	//Create the socket that we'll be listening on...
	socket_int = new genericSocketInterface(in_socket_type, socket_num);

	//Link upper and lower 
	in_sdr_int->addLowerLevel(this);
	socket_int->addUpperLevel(this);
}

portalCommandSocket::~portalCommandSocket(){
	delete socket_int;
}

void portalCommandSocket::dataFromUpperLevel(void *data, int num_bytes, int local_up_channel=0){
	//Data coming in from the SDR

	messageType new_message;
	new_message.buffer = data;
	new_message.num_bytes = num_bytes;

	vector<messageType> transmit_messages;
	transmit_messages.push_back(new_message);

	socket_int->dataFromUpperLevel(&transmit_messages, num_bytes);
}

void portalCommandSocket::dataFromLowerLevel(void *data, int num_bytes, int local_down_channel=0){
	//Data coming in from the socket

	//First, we need to make sure data is casted correctly (data is a pointer to a vector of messages)
	vector<messageType> *in_messages = static_cast<vector<messageType>*>(data);

	//Insert historic messages into a string stream so as to easily extract lines
	static stringstream command_stream;
	for(int ii=0; ii < in_messages->size(); ii++){
		string in_data_string((*in_messages)[ii].buffer,(*in_messages)[ii].num_bytes);
		command_stream << in_data_string;
	}

	//Now parse out incoming commands
	string current_command;
	if(!getline(command_stream, current_command).fail()){
		stringstream arg_stream(current_command);
		string command, arg1, arg2;

		//Process the command, first argument, and second argument
		arg_stream >> command;
		arg_stream >> arg1;
		arg_stream >> arg2;

		cout << command << " " << arg1 << " " << arg2 << endl;

		//Now do whatever we need to do based on the received command
		//TODO: Put in some error checking here
		try{
			if(command == "CHANNEL"){
				if(isInteger(arg1)){
					int candidate_channel = strtol(arg1.c_str(), NULL, 0);
					if(uhd_int->getNumAllocatedChannels() > candidate_channel)
						cur_channel = candidate_channel;
					else
						throw badArgumentException(badArgumentException::OUT_OF_BOUNDS, 1, arg1);
				} else
					throw badArgumentException(badArgumentException::MALFORMED, 1, arg1);
			} else if(command == "RXCHANNEL"){
				if(isInteger(arg1))
					if(isInteger(arg2))
						cur_channel = uhd_int->getRXPortUID(strtol(arg1.c_str(), NULL, 0));
						//TODO: Respond with UID
					else
						throw badArgumentException(badArgumentException::MALFORMED, 2, arg2);
				else
					throw badArgumentException(badArgumentException::MALFORMED, 1, arg1);
			} else if(command == "TXCHANNEL"){
				if(isInteger(arg1))
					if(isInteger(arg2))
						cur_channel = uhd_int->getTXPortUID(strtol(arg1.c_str(), NULL, 0));
						//TODO: Respond with UID
					else
						throw badArgumentException(badArgumentException::MALFORMED, 2, arg2);
				else
					throw badArgumentException(badArgumentException::MALFORMED, 1, arg1);
			} else {
				uhd_int->setSDRParameter(command, arg1);
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
			stringstream response;
			response << "?" << e.what() << endl;
			upstream_int->dataFromUpstream(response.str().c_str(),response.gcount(),uhd_int);
		} catch(invalidCommandException const& e){
			stringstream response;
			response << "?" << e.what() << endl;
			upstream_int->dataFromUpstream(response.str().c_str(),response.gcount(),uhd_int);
		}

		//Query-based commands
		char response[20];
		int response_length;
		if(command.substr(0,6) == "RXFREQ")
			response_length = sprintf(response,"%d\r\n",(int)(uhd_int->getUHDObject()->get_rx_freq()+0.5));
		else if(command.substr(0,6) == "TXFREQ")
			response_length = sprintf(response,"%d\r\n",(int)(uhd_int->getUHDObject()->get_tx_freq()+0.5));
		else if(command.substr(0,6) == "RXGAIN")
			response_length = sprintf(response,"%f\r\n",uhd_int->getUHDObject()->get_rx_gain());
		else if(command.substr(0,6) == "TXGAIN")
			response_length = sprintf(response,"%f\r\n",uhd_int->getUHDObject()->get_tx_gain());
		else if(command.substr(0,6) == "RXRATE")
			response_length = sprintf(response,"%d\r\n",(int)(uhd_int->getUHDObject()->get_rx_rate()+0.5));
		else if(command.substr(0,6) == "TXRATE")
			response_length = sprintf(response,"%d\r\n",(int)(uhd_int->getUHDObject()->get_tx_rate()+0.5));
		else if(command.substr(0,5) == "RXANT")
			response_length = sprintf(response,"%s\r\n",uhd_int->getUHDObject()->get_rx_antenna().c_str());
		else if(command.substr(0,5) == "TXANT")
			response_length = sprintf(response,"%s\r\n",uhd_int->getUHDObject()->get_tx_antenna().c_str());

		//Send off the response
		//TODO: This upstream object is different now...
		upstream_int->dataFromUpstream(response,response_length,uhd_int);
	}

}

