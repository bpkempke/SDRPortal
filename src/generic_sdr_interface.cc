#include <generic_sdr_interface.h>

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
	uid_map[in_channel] = num_channels;
	in_channel->setUID(num_channels++);

	return num_channels;
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
			paramData val_double;
			if(isInteger(val))
				val_double = paramData(strtod(val.c_str(), NULL), uid_to_chaninfo[in_uid]);
			else
				throw badArgumentException(badArgumentException::MALFORMED, 1, val);

			//Check to make sure the value is within bounds
			if(!(this->*(cur_param_details.checkMethod))(val_double))
				throw badArgumentException(badArgumentException::OUT_OF_BOUNDS, 1, val);

			(this->*(cur_param_details.setMethod))(val_double);
		} else {
			//TODO: Any other parameter types we want to support?
		}
	}
}

void genericSDRInterface::bindRXChannel(int rx_chan, int in_uid){
	//Check to see if the RX channel is an actual channel
	if(checkRXChannel(rx_chan)){
		//Open the RX channel since it's valid (don't care if it's been opened before)
		openRXChannel(rx_chan);
		rx_chan_to_streams[rx_chan].push_back(uid_map.find(in_uid));
		uid_to_chaninfo[in_uid].rx_chan = rx_chan;
	} else {
		throw badArgumentException(badArgumentException::MALFORMED, 1, val); //TODO: is this the correct exception to throw?
	}
}

void genericSDRInterface::bindTXChannel(int tx_chan, int in_uid){
	//Check to see if the RX channel is an actual channel
	if(checkTXChannel(tx_chan)){
		//Open the RX channel since it's valid (don't care if it's been opened before)
		openTXChannel(tx_chan);
		tx_chan_to_streams[tx_chan].push_back(uid_map.find(in_uid));
		uid_to_chaninfo[in_uid].tx_chan = tx_chan;
	} else {
		throw badArgumentException(badArgumentException::MALFORMED, 1, val); //TODO: is this the correct exception to throw?
	}
}

int genericSDRInterface::getNumAllocatedChannels(){
	return num_channels;
}

vector<primType> genericSDRInterface::getResultingPrimTypes(int rx_chan){
	vector<primType> resulting_prim_types;
	vector<portalDataSocket*> rx_streams = rx_chan_to_streams[rx_chan];
	for(int ii=0; ii < rx_streams.size(); ii++){
		primType cur_prim_type = rx_streams[ii]->getDataType();
		vector<primType>::iterator it = find(resulting_prim_types.begin(), resulting_prim_types.end(), cur_prim_type);
		if(it == resulting_prim_types.end())
			resulting_prim_types.push_back(cur_prim_type);
	}
	return resulting_prim_types;
}

//TODO: How do we do any type changes?
void genericSDRInterface::distributeRXData(void *in_data, int num_bytes, int rx_chan){
	//Data coming from SDR RX for distribution to sockets
	vector<hierarchicalDataflowBlock*> streams = rx_chan_to_streams[rx_chan];
	for(int ii=0; ii < streams.size(); ii++)
		streams[ii].dataFromUpperLevel(in_data, num_bytes);
}

void genericSDRInterface::sendIQData(void *in_data, int num_bytes, int uid_port){
	//Data coming from socket for TX
	int tx_chan = uid_to_chaninfo[uid_port].tx_chan;
	txIQData(in_data, num_bytes, tx_chan);
}
