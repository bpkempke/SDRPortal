#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include "portalDataSocket.h"
#include "genericSDRInterface.h"

//TODO: Integrate PPM correction into genericSDRInterface
//TODO: Integrate DC offset correction into genericSDRInterface

genericSDRInterface::genericSDRInterface(){
	num_channels = 0;
	cur_channel = 0;

	//First, register all of the parameters which can be modified and the accessor methods which deal with them
	param_accessors["RXFREQ"] = paramAccessor(DOUBLE, &genericSDRInterface::setRXFreq, &genericSDRInterface::getRXFreq, &genericSDRInterface::checkRXFreq);
	param_accessors["TXFREQ"] = paramAccessor(DOUBLE, &genericSDRInterface::setTXFreq, &genericSDRInterface::getTXFreq, &genericSDRInterface::checkTXFreq);
	param_accessors["RXGAIN"] = paramAccessor(DOUBLE, &genericSDRInterface::setRXGain, &genericSDRInterface::getRXGain, &genericSDRInterface::checkRXGain);
	param_accessors["TXGAIN"] = paramAccessor(DOUBLE, &genericSDRInterface::setTXGain, &genericSDRInterface::getTXGain, &genericSDRInterface::checkTXGain);
	param_accessors["RXRATE"] = paramAccessor(DOUBLE, &genericSDRInterface::setRXRate, &genericSDRInterface::getRXRate, &genericSDRInterface::checkRXRate);
	param_accessors["TXRATE"] = paramAccessor(DOUBLE, &genericSDRInterface::setTXRate, &genericSDRInterface::getTXRate, &genericSDRInterface::checkTXRate);

}

int genericSDRInterface::addChannel(portalDataSocket *in_channel){
	
	//Also create a sequential UID for this port as well
	uid_map[num_channels] = in_channel;
	uid_to_chaninfo[num_channels] = rxtxChanInfo(-1, -1);
	in_channel->setUID(num_channels++);

	return num_channels-1;
}

void genericSDRInterface::setStreamDataType(primType in_type, int in_uid){

	//First make sure that this UID is actually associated with something...
	std::map<int,portalDataSocket*>::iterator found_data_socket = uid_map.find(in_uid);

	//If that data socket is found, set its type, otherwise throw an error
	if(found_data_socket != uid_map.end()){
		portalDataSocket *cur_data_socket = found_data_socket->second;
		cur_data_socket->setDataType(in_type);
	} else throw badArgumentException(badArgumentException::OUT_OF_BOUNDS, 1, std::to_string(in_uid));
}

void genericSDRInterface::setSDRParameter(int in_uid, std::string name, std::string val){
	//This function dynamically checks parameters of differing types and then loads into an abstract primitive container to avoid double dispatch
	typename std::map<std::string, paramAccessor >::iterator it = param_accessors.find(name);
	if(it == param_accessors.end()){
		//Didn't find the command in the list of those supported, so throw an error...
		throw invalidCommandException(name);
	} else {
		paramAccessor cur_param_details = param_accessors[name];

		//Determine which type of argument this command accepts, and perform different checks depending on that type
		if(cur_param_details.arg_type == INT){
			paramData val_int;
			if(isInteger(val))
				val_int = paramData((int)(strtol(val.c_str(), NULL, 0)), uid_to_chaninfo[in_uid]);
			else
				throw badArgumentException(badArgumentException::MALFORMED, 1, val);

			//Check to make sure the value is within bounds
			if(!(this->*(cur_param_details.checkMethod))(val_int))
				throw badArgumentException(badArgumentException::OUT_OF_BOUNDS, 1, val);

			(this->*cur_param_details.setMethod)(val_int);
		} else if(cur_param_details.arg_type == DOUBLE){
			if(isDouble(val)){
				paramData val_double(strtod(val.c_str(), NULL), uid_to_chaninfo[in_uid]);
				//Check to make sure the value is within bounds
				//if(!(this->*(cur_param_details.checkMethod))(val_double))
				//	throw badArgumentException(badArgumentException::OUT_OF_BOUNDS, 1, val);

				(this->*(cur_param_details.setMethod))(val_double);
			} else
				throw badArgumentException(badArgumentException::MALFORMED, 1, val);

		} else if(cur_param_details.arg_type == C_STRING){
			paramData val_cstr(val.c_str(), uid_to_chaninfo[in_uid]);
			(this->*(cur_param_details.setMethod))(val_cstr);
		} else {
			//SHOULDN'T GET HERE???
		}
	}
}

void genericSDRInterface::bindRXChannel(int rx_chan, int in_uid){
	//Check to see if the RX channel is an actual channel
	if(checkRXChannel(rx_chan)){
		//Open the RX channel since it's valid (don't care if it's been opened before)
		openRXChannel(rx_chan);
		if(rx_chan_to_streams.count(rx_chan) == 0)
			rx_chan_to_streams[rx_chan] = std::vector<portalDataSocket*>();
		rx_chan_to_streams[rx_chan].push_back(uid_map[in_uid]);
		uid_to_chaninfo[in_uid].rx_chan = rx_chan;
	} else {
		std::stringstream res;
		res << rx_chan;
		throw badArgumentException(badArgumentException::MALFORMED, 1, res.str());
	}
}

void genericSDRInterface::bindTXChannel(int tx_chan, int in_uid){
	//Check to see if the RX channel is an actual channel
	if(checkTXChannel(tx_chan)){
		//Open the RX channel since it's valid (don't care if it's been opened before)
		openTXChannel(tx_chan);
		tx_chan_to_streams[tx_chan].push_back(uid_map[in_uid]);
		uid_to_chaninfo[in_uid].tx_chan = tx_chan;
	} else {
		std::stringstream res;
		res << tx_chan;
		throw badArgumentException(badArgumentException::MALFORMED, 1, res.str());
	}
}

int genericSDRInterface::getNumAllocatedChannels(){
	return num_channels;
}

std::vector<primType> genericSDRInterface::getResultingPrimTypes(int rx_chan){
	std::vector<primType> resulting_prim_types;
	std::vector<portalDataSocket*> rx_streams = rx_chan_to_streams[rx_chan];
	for(unsigned int ii=0; ii < rx_streams.size(); ii++){
		primType cur_prim_type = rx_streams[ii]->getDataType();
		std::vector<primType>::iterator it = std::find(resulting_prim_types.begin(), resulting_prim_types.end(), cur_prim_type);
		if(it == resulting_prim_types.end())
			resulting_prim_types.push_back(cur_prim_type);
	}
	return resulting_prim_types;
}

void genericSDRInterface::distributeRXData(void *in_data, int num_bytes, int rx_chan, primType in_type){
	//Data coming from SDR RX for distribution to sockets
	std::vector<portalDataSocket*> streams = rx_chan_to_streams[rx_chan];
	for(unsigned int ii=0; ii < streams.size(); ii++)
		if(streams[ii]->getDataType() == in_type){
//			std::cout << "Distributing data to streams..." << std::endl;
			streams[ii]->dataFromUpperLevel(in_data, num_bytes);
		}
}

rxtxChanInfo genericSDRInterface::getChanInfo(int in_uid){
	return uid_to_chaninfo[in_uid];
}

void genericSDRInterface::dataFromLowerLevel(void *data, int num_messages, int local_down_channel){
	//Data coming from socket for TX

	messageType *in_messages = static_cast<messageType*>(data);
	for(int ii=0; ii < num_messages; ii++){
		int tx_chan = uid_to_chaninfo[in_messages[ii].socket_channel].tx_chan;
		txIQData((void*)in_messages[ii].buffer, in_messages[ii].num_bytes, tx_chan, in_messages[ii].data_type);
	}
}

