
void genericSDRInterface::setSDRParameter(std::string name, std::string val){
	//This function dynamically checks parameters of differing types and then loads into an abstract primitive container to avoid double dispatch
	std::map<std::string, paramDetails>::iterator it = param_accessors.find(name);
	if(it == param_accessors.end()){
		//Didn't find the command in the list of those supported, so throw an error...
		throw invalidCommandException(name);
	} else {
		paramDetails cur_param_details = param_accessors[name];

		//Determine which type of argument this command accepts, and perform different checks depending on that type
		if(cur_param_details.arg_type == INT){
			paramData val_int;
			if(isInteger(val))
				val_int = paramData((int)(strtol(arg.c_str(), NULL)));
			else
				throw badArgumentException(MALFORMED, val);

			//Check to make sure the value is within bounds
			if(!(this->*(cur_param_details.checkMethod))(val_int))
				throw badArgumentException(OUT_OF_BOUNDS, val);

			(this->*(cur_param_details.setMethod))(val_int);
		} else if(cur_param_details.arg_type == DOUBLE){
			paramData val_double;
			if(isInteger(val))
				val_double = paramData(strtod(arg.c_str(), NULL));
			else
				throw badArgumentException(MALFORMED, val);

			//Check to make sure the value is within bounds
			if(!(this->*(cur_param_details.checkMethod))(val_double))
				throw badArgumentException(OUT_OF_BOUNDS, val);

			(this->*(cur_param_details.setMethod))(val_double);
		} else {
			//TODO: Any other parameter types we want to support?
		}
	}
}

int genericSDRInterface::getRXPortUID(int rx_port){
	//First try to see if this port's been given a UID
	std::map<int, int>::iterator it = rx_to_uid.find(rx_port);
	if(it != rx_to_uid.end())
		return rx_to_uid[rx_port];
	else {
		int ret_uid = cur_uid;
		rx_to_uid[rx_port] = cur_uid;
		return ret_uid;
	}
}

int genericSDRInterface::getTXPortUID(int tx_port){
	//First try to see if this port's been given a UID
	std::map<int, int>::iterator it = rx_to_uid.find(rx_port);
	if(it != rx_to_uid.end())
		return rx_to_uid[rx_port];
	else {
		int ret_uid = cur_uid;
		rx_to_uid[rx_port] = cur_uid;
		return ret_uid;
	}
}

int genericSDRInterface::getGenericPortUID(int generic_port){
	//First try to see if this port's been given a UID
	std::map<int, int>::iterator it = generic_to_uid.find(rx_port);
	if(it != generic_to_uid.end())
		return generic_to_uid[rx_port];
	else {
		int ret_uid = cur_uid;
		generic_to_uid[rx_port] = cur_uid;
		return ret_uid;
	}
}
