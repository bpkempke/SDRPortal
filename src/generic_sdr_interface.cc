#include <generic.h>
#include <generic_sdr_interface.h>

template<class T>
genericSDRInterface<T>::genericSDRInterface(){
	cur_uid = 0;
}

template<class T>
void genericSDRInterface<T>::setSDRParameter(std::string name, std::string val){
	//This function dynamically checks parameters of differing types and then loads into an abstract primitive container to avoid double dispatch
	typename std::map<std::string, paramAccessor<T> >::iterator it = param_accessors.find(name);
	if(it == param_accessors.end()){
		//Didn't find the command in the list of those supported, so throw an error...
		throw invalidCommandException(name);
	} else {
		paramAccessor<T> cur_param_details = param_accessors[name];

		//Determine which type of argument this command accepts, and perform different checks depending on that type
		if(cur_param_details.arg_type == INT){
			paramData val_int;
			if(isInteger(val))
				val_int = paramData((int)(strtol(val.c_str(), NULL, 0)));
			else
				throw badArgumentException(badArgumentException::MALFORMED, val);

			//Check to make sure the value is within bounds
			if(!(this->*(cur_param_details.checkMethod))(val_int))
				throw badArgumentException(badArgumentException::OUT_OF_BOUNDS, val);

			(this->*(cur_param_details.setMethod))(val_int);
		} else if(cur_param_details.arg_type == DOUBLE){
			paramData val_double;
			if(isInteger(val))
				val_double = paramData(strtod(val.c_str(), NULL));
			else
				throw badArgumentException(badArgumentException::MALFORMED, val);

			//Check to make sure the value is within bounds
			if(!(this->*(cur_param_details.checkMethod))(val_double))
				throw badArgumentException(badArgumentException::OUT_OF_BOUNDS, val);

			(this->*(cur_param_details.setMethod))(val_double);
		} else {
			//TODO: Any other parameter types we want to support?
		}
	}
}

template<class T>
int genericSDRInterface<T>::getRXPortUID(int rx_port){
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

template<class T>
int genericSDRInterface<T>::getTXPortUID(int tx_port){
	//First try to see if this port's been given a UID
	std::map<int, int>::iterator it = tx_to_uid.find(tx_port);
	if(it != tx_to_uid.end())
		return tx_to_uid[tx_port];
	else {
		int ret_uid = cur_uid;
		tx_to_uid[tx_port] = cur_uid;
		return ret_uid;
	}
}

template<class T>
int genericSDRInterface<T>::getGenericPortUID(int generic_port){
	//First try to see if this port's been given a UID
	std::map<int, int>::iterator it = generic_to_uid.find(generic_port);
	if(it != generic_to_uid.end())
		return generic_to_uid[generic_port];
	else {
		int ret_uid = cur_uid;
		generic_to_uid[generic_port] = cur_uid;
		return ret_uid;
	}
}
