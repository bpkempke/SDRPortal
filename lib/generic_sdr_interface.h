#include <map>
#include <string>
#include <string.h>
#include <generic.h>

#ifndef GENERIC_SDR_INT_H
#define GENERIC_SDR_INT_H

class paramData {
public:
	paramData(double in_data, int in_channel=0){scratch = new double(in_data); channel = in_channel;};
	paramData(int in_data, int in_channel=0){scratch = new int(in_data); channel = in_channel;};
	paramData(){scratch = NULL;};
	~paramData(){if(scratch) delete scratch;};//TODO: Fix this warning...

	//Accessor methods
	int getInt(){int ret_val; memcpy(&ret_val, scratch, sizeof(int)); return ret_val;};
	int getDouble(){double ret_val; memcpy(&ret_val, scratch, sizeof(double)); return ret_val;};
	int getChannel(){return channel;};
private:
	void *scratch;
	int channel;
};

enum primEnum {DOUBLE, INT, VOID};

template<class T>
struct paramAccessor {
	//typdefs to make things prettier later on...
	typedef void (T::*setMethodType)(paramData);
	typedef paramData (T::*getMethodType)(int);
	typedef bool (T::*checkMethodType)(paramData);

	//actual struct members (using camel-caps so that the use of these function pointers later-on looks cleaner)
	primEnum arg_type;
	setMethodType setMethod;
	getMethodType getMethod;
	checkMethodType checkMethod;

	//struct constructor
	paramAccessor(primEnum in_prim, setMethodType in_set, getMethodType in_get, checkMethodType in_check) : arg_type(in_prim), setMethod(in_set), getMethod(in_get), checkMethod(in_check) {};
	paramAccessor(){};
};

template<class T>
class genericSDRInterface {
public:
	genericSDRInterface<T>();
	void setSDRParameter(std::string name, std::string val);
	void sendIQData(void *data, int num_elements, int uid_port);
	int getRXPortUID(int rx_port);
	int getTXPortUID(int tx_port);
	int getGenericPortUID(int generic_port);
	int getNumAllocatedChannels();
protected:
	virtual void setRXFreq(paramData in_param) = 0;
	virtual void setTXFreq(paramData in_param) = 0;
	virtual void setRXGain(paramData in_param) = 0;
	virtual void setTXGain(paramData in_param) = 0;
	virtual void setRXRate(paramData in_param) = 0;
	virtual void setTXRate(paramData in_param) = 0;
	virtual paramData getRXFreq(int in_chan) = 0;
	virtual paramData getTXFreq(int in_chan) = 0;
	virtual paramData getRXGain(int in_chan) = 0;
	virtual paramData getTXGain(int in_chan) = 0;
	virtual paramData getRXRate(int in_chan) = 0;
	virtual paramData getTXRate(int in_chan) = 0;
	virtual bool checkRXChannel(int in_chan) = 0;
	virtual bool checkTXChannel(int in_chan) = 0;
	virtual bool checkRXFreq(paramData in_param) = 0;
	virtual bool checkTXFreq(paramData in_param) = 0;
	virtual bool checkRXGain(paramData in_param) = 0;
	virtual bool checkTXGain(paramData in_param) = 0;
	virtual bool checkRXRate(paramData in_param) = 0;
	virtual bool checkTXRate(paramData in_param) = 0;
	virtual void setCustomSDRParameter(std::string name, std::string val, int in_chan) = 0;
	virtual std::string getCustomSDRParameter(std::string name) = 0;
	virtual void setStreamDataType(streamType in_type) = 0;
	std::map<std::string, paramAccessor<T> > param_accessors;
private:
	int cur_uid;
	std::map<int, int> rx_to_uid;
	std::map<int, int> tx_to_uid;
	std::map<int, int> generic_to_uid;
};

template<class T>
genericSDRInterface<T>::genericSDRInterface(){
	cur_uid = 0;
}

template<class T>
void genericSDRInterface<T>::setSDRParameter(std::string name, std::string val){
	//Pointer to derived class
	T *derivedClass = static_cast<T*>(this);

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
				throw badArgumentException(badArgumentException::MALFORMED, 1, val);

			//Check to make sure the value is within bounds
			if(!(derivedClass->*(cur_param_details.checkMethod))(val_int))
				throw badArgumentException(badArgumentException::OUT_OF_BOUNDS, 1, val);

			(derivedClass->*cur_param_details.setMethod)(val_int);
		} else if(cur_param_details.arg_type == DOUBLE){
			paramData val_double;
			if(isInteger(val))
				val_double = paramData(strtod(val.c_str(), NULL));
			else
				throw badArgumentException(badArgumentException::MALFORMED, 1, val);

			//Check to make sure the value is within bounds
			if(!(derivedClass->*(cur_param_details.checkMethod))(val_double))
				throw badArgumentException(badArgumentException::OUT_OF_BOUNDS, 1, val);

			(derivedClass->*(cur_param_details.setMethod))(val_double);
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

template<class T>
int genericSDRInterface<T>::getNumAllocatedChannels(){
	return cur_uid;
}

#endif
