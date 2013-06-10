#include <map>
#include <string>
#include <string.h>
#include <generic.h>

#ifndef GENERIC_SDR_INT_H
#define GENERIC_SDR_INT_H

enum primEnum {DOUBLE, INT, VOID};
enum cPrimType {C_DOUBLE64, C_FLOAT32, C_INT32, C_INT16, C_INT8};

struct rxtxChanInfo{
	int rx_chan;
	int tx_chan;
	rxtxChanInfo(int r, int t) : rx_chan(r), tx_chan(t) {};
};

class paramData {
public:
	paramData(double in_data, rxtxChanInfo in_channel=0){data_type = DOUBLE; scratch = new double(in_data); channel = in_channel;};
	paramData(int in_data, rxtxChanInfo in_channel=0){data_type = INT; scratch = new int(in_data); channel = in_channel;};
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
	rxtxChanInfo channel;
};

struct iqData {
	void *data;
	int num_bytes;
	int channel_uid;
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

class genericSDRInterface{
public:
	genericSDRInterface();
	void setSDRParameter(int in_uid, std::string name, std::string val);
	int getNumAllocatedChannels();

	int addChannel(portalDataSocket *in_channel);
	void bindRXChannel(int rx_chan, int in_uid);
	void bindTXChannel(int tx_chan, int in_uid);

	vector<primType> getResultingPrimTypes(int rx_chan);
	void distributeRXData(void *in_data, int num_bytes, int rx_chan, primType in_type);
	void sendIQData(void *data, int num_bytes, int uid_port, primType in_type);
	void setStreamDataType(streamType in_type); //TODO: Need to integrate this in

protected:
	//All possible get/set/check methods.  If they're not implemented, the virtual method will default to throwing an exception
	virtual void setRXFreq(paramData in_param){throw invalidCommandException("");};
	virtual void setTXFreq(paramData in_param){throw invalidCommandException("");};
	virtual void setRXGain(paramData in_param){throw invalidCommandException("");};
	virtual void setTXGain(paramData in_param){throw invalidCommandException("");};
	virtual void setRXRate(paramData in_param){throw invalidCommandException("");};
	virtual void setTXRate(paramData in_param){throw invalidCommandException("");};
	virtual paramData getRXFreq(rxtxChanInfo in_chan){throw invalidCommandException("");};
	virtual paramData getTXFreq(rxtxChanInfo in_chan){throw invalidCommandException("");};
	virtual paramData getRXGain(rxtxChanInfo in_chan){throw invalidCommandException("");};
	virtual paramData getTXGain(rxtxChanInfo in_chan){throw invalidCommandException("");};
	virtual paramData getRXRate(rxtxChanInfo in_chan){throw invalidCommandException("");};
	virtual paramData getTXRate(rxtxChanInfo in_chan){throw invalidCommandException("");};
	virtual bool checkRXChannel(int in_chan){throw invalidCommandException("");};
	virtual bool checkTXChannel(int in_chan){throw invalidCommandException("");};
	virtual void openRXChannel(int in_chan){throw invalidCommandException("");};
	virtual void openTXChannel(int in_chan){throw invalidCommandException("");};
	virtual bool checkRXFreq(paramData in_param){throw invalidCommandException("");};
	virtual bool checkTXFreq(paramData in_param){throw invalidCommandException("");};
	virtual bool checkRXGain(paramData in_param){throw invalidCommandException("");};
	virtual bool checkTXGain(paramData in_param){throw invalidCommandException("");};
	virtual bool checkRXRate(paramData in_param){throw invalidCommandException("");};
	virtual bool checkTXRate(paramData in_param){throw invalidCommandException("");};
	virtual void setCustomSDRParameter(std::string name, std::string val, int in_chan) = 0;
	virtual void txIQData(void *data, int num_bytes, int tx_chan, primType in_type) = 0;
	std::map<std::string, paramAccessor > param_accessors;
private:
	int num_channels, cur_channel;
	map<portalDataSocket*,int> uid_map;
	map<int,vector<portalDataSocket*> > rx_chan_to_streams, tx_chan_to_streams;
	map<int,rxtxChanInfo> uid_to_chaninfo;
};

#endif
