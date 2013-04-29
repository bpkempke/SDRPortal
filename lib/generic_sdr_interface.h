#include <map>
#include <string>
#include <string.h>
#include <generic.h>

#ifndef GENERIC_SDR_INT_H
#define GENERIC_SDR_INT_H

enum primEnum {DOUBLE, INT, VOID};

class paramData {
public:
	paramData(double in_data, int in_channel=0){data_type = DOUBLE; scratch = new double(in_data); channel = in_channel;};
	paramData(int in_data, int in_channel=0){data_type = INT; scratch = new int(in_data); channel = in_channel;};
	paramData(){data_type = VOID;};
	~paramData(){
		if(data_type == DOUBLE) delete (double*)scratch;
		else if(data_type == INT) delete (int*)scratch;};

	//Accessor methods
	int getInt(){int ret_val; memcpy(&ret_val, scratch, sizeof(int)); return ret_val;};
	int getDouble(){double ret_val; memcpy(&ret_val, scratch, sizeof(double)); return ret_val;};
	int getChannel(){return channel;};
private:
	void *scratch;
	primEnum data_type;
	int channel;
};

struct paramAccessor {
	//typdefs to make things prettier later on...
	typedef void (genericSDRInterface::*setMethodType)(paramData);
	typedef paramData (genericSDRInterface::*getMethodType)(int);
	typedef bool (genericSDRInterface::*checkMethodType)(paramData);

	//actual struct members (using camel-caps so that the use of these function pointers later-on looks cleaner)
	primEnum arg_type;
	setMethodType setMethod;
	getMethodType getMethod;
	checkMethodType checkMethod;

	//struct constructor
	paramAccessor(primEnum in_prim, setMethodType in_set, getMethodType in_get, checkMethodType in_check) : arg_type(in_prim), setMethod(in_set), getMethod(in_get), checkMethod(in_check) {};
	paramAccessor(){};
};

class genericSDRInterface {
public:
	genericSDRInterface();
	void setSDRParameter(std::string name, std::string val);
	void sendIQData(void *data, int num_elements, int uid_port);
	int getRXPortUID(int rx_port);
	int getTXPortUID(int tx_port);
	int getGenericPortUID(int generic_port);
	int getNumAllocatedChannels();
protected:
	//All possible get/set/check methods.  If they're not implemented, the virtual method will default to throwing an exception
	virtual void setRXFreq(paramData in_param){throw invalidCommandException("");};
	virtual void setTXFreq(paramData in_param){throw invalidCommandException("");};
	virtual void setRXGain(paramData in_param){throw invalidCommandException("");};
	virtual void setTXGain(paramData in_param){throw invalidCommandException("");};
	virtual void setRXRate(paramData in_param){throw invalidCommandException("");};
	virtual void setTXRate(paramData in_param){throw invalidCommandException("");};
	virtual paramData getRXFreq(int in_chan){throw invalidCommandException("");};
	virtual paramData getTXFreq(int in_chan){throw invalidCommandException("");};
	virtual paramData getRXGain(int in_chan){throw invalidCommandException("");};
	virtual paramData getTXGain(int in_chan){throw invalidCommandException("");};
	virtual paramData getRXRate(int in_chan){throw invalidCommandException("");};
	virtual paramData getTXRate(int in_chan){throw invalidCommandException("");};
	virtual bool checkRXChannel(int in_chan){throw invalidCommandException("");};
	virtual bool checkTXChannel(int in_chan){throw invalidCommandException("");};
	virtual bool checkRXFreq(paramData in_param){throw invalidCommandException("");};
	virtual bool checkTXFreq(paramData in_param){throw invalidCommandException("");};
	virtual bool checkRXGain(paramData in_param){throw invalidCommandException("");};
	virtual bool checkTXGain(paramData in_param){throw invalidCommandException("");};
	virtual bool checkRXRate(paramData in_param){throw invalidCommandException("");};
	virtual bool checkTXRate(paramData in_param){throw invalidCommandException("");};
	virtual void setCustomSDRParameter(std::string name, std::string val, int in_chan) = 0;
	//virtual std::string getCustomSDRParameter(std::string name) = 0;
	virtual void setStreamDataType(streamType in_type) = 0;
	std::map<std::string, paramAccessor > param_accessors;
private:
	int cur_uid;
	std::map<int, int> rx_to_uid;
	std::map<int, int> tx_to_uid;
	std::map<int, int> generic_to_uid;
};

#endif
